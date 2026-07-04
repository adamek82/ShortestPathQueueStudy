#include "search.hpp"

#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace spq {
namespace {

int scoreStats(const BfsStats& stats, bool requireReachable) {
    if (requireReachable && !stats.reachedTarget)
        return -1;

    return stats.maxQueueSize;
}

unsigned int effectiveSeed(unsigned int seed) {
    if (seed != 0)
        return seed;

    return std::random_device{}();
}

void validateConfig(const SearchConfig& config) {
    if (config.n < 2)
        throw std::invalid_argument("n must be at least 2");

    if (config.iterations <= 0)
        throw std::invalid_argument("iterations must be positive");

    if (config.restarts <= 0)
        throw std::invalid_argument("restarts must be positive");

    if (config.initialBlockProbability < 0.0 || config.initialBlockProbability > 1.0)
        throw std::invalid_argument("density must be in [0, 1]");
}

void publishProgress(const SearchConfig& config) {
    if (config.progressCallback)
        config.progressCallback(1);
}

void publishBest(const SearchConfig& config, const SearchResult& best) {
    if (config.bestCallback)
        config.bestCallback(best);
}

void considerBest(const SearchResult& candidate, SearchResult& best,
                  const SearchConfig& config) {
    if (!isBetterSearchResult(candidate, best))
        return;

    best = candidate;
    publishBest(config, best);
}

SearchResult evaluateGrid(Grid grid, bool requireReachable, int restart, int iteration,
                          unsigned int seed) {
    SearchResult result;
    result.stats = analyzeShortestPathBinaryMatrix(grid);
    result.score = scoreStats(result.stats, requireReachable);
    result.grid = std::move(grid);
    result.restart = restart;
    result.iteration = iteration;
    result.seed = seed;
    return result;
}

SearchResult makeInitialCandidate(const SearchConfig& config, std::mt19937& rng,
                                  unsigned int seed, int restart) {
    SearchResult last;

    constexpr int kAttempts = 1000;
    for (int attempt = 0; attempt < kAttempts; ++attempt) {
        Grid grid = makeRandomGrid(config.n, config.initialBlockProbability, rng);
        SearchResult candidate = evaluateGrid(std::move(grid), config.requireReachable,
                                              restart, 0, seed);

        last = candidate;

        if (!config.requireReachable || candidate.score >= 0)
            return candidate;
    }

    return last;
}

std::string formatSeconds(double seconds) {
    std::ostringstream os;

    if (seconds < 60.0) {
        os << std::fixed << std::setprecision(1) << seconds << "s";
        return os.str();
    }

    const int minutes = static_cast<int>(seconds / 60.0);
    const int restSeconds = static_cast<int>(std::fmod(seconds, 60.0));

    os << minutes << "m " << restSeconds << "s";
    return os.str();
}

class ProgressPrinter {
public:
    ProgressPrinter(int totalIterations, int progressEvery, bool verbose)
        : totalIterations_(totalIterations),
          progressEvery_(progressEvery),
          verbose_(verbose),
          start_(std::chrono::steady_clock::now()) {}

    void maybePrint(int done, const SearchResult& best, int currentScore, int restart,
                    int iteration) const {
        if (!verbose_ || progressEvery_ <= 0)
            return;

        const bool shouldPrint = done == totalIterations_ || done % progressEvery_ == 0;
        if (!shouldPrint)
            return;

        const auto now = std::chrono::steady_clock::now();
        const double elapsed = std::chrono::duration<double>(now - start_).count();

        const double ratio = done > 0 ? static_cast<double>(done) / totalIterations_ : 0.0;
        const double totalEstimate = ratio > 0.0 ? elapsed / ratio : 0.0;
        const double eta = totalEstimate > elapsed ? totalEstimate - elapsed : 0.0;

        std::cout << "restart=" << restart
                  << " iter=" << iteration
                  << " done=" << done << "/" << totalIterations_
                  << " current=" << currentScore
                  << " best=" << best.score
                  << " bestPath=" << best.stats.shortestPath
                  << " elapsed=" << formatSeconds(elapsed)
                  << " eta=" << formatSeconds(eta)
                  << '\n';
    }

private:
    int totalIterations_;
    int progressEvery_;
    bool verbose_;
    std::chrono::steady_clock::time_point start_;
};

double temperatureAt(const AnnealingConfig& config, int iteration) {
    if (config.iterations <= 1)
        return config.endTemperature;

    const double t = static_cast<double>(iteration - 1) /
                     static_cast<double>(config.iterations - 1);

    return config.startTemperature *
           std::pow(config.endTemperature / config.startTemperature, t);
}

} // namespace

bool isBetterSearchResult(const SearchResult& lhs, const SearchResult& rhs) {
    if (lhs.score != rhs.score)
        return lhs.score > rhs.score;

    if (lhs.stats.shortestPath != rhs.stats.shortestPath)
        return lhs.stats.shortestPath > rhs.stats.shortestPath;

    return lhs.stats.visitedCells > rhs.stats.visitedCells;
}

SearchResult randomSearch(const SearchConfig& config) {
    validateConfig(config);

    const unsigned int seed = effectiveSeed(config.seed);
    std::mt19937 rng(seed);

    SearchResult best;
    best.seed = seed;

    const int totalIterations = config.iterations * config.restarts;
    ProgressPrinter progress(totalIterations, config.progressEvery, config.verbose);

    int done = 0;

    for (int restart = 1; restart <= config.restarts; ++restart) {
        const int reportedRestart = config.restartOffset + restart;

        for (int iteration = 1; iteration <= config.iterations; ++iteration) {
            Grid grid = makeRandomGrid(config.n, config.initialBlockProbability, rng);
            SearchResult candidate = evaluateGrid(std::move(grid), config.requireReachable,
                                                  reportedRestart, iteration, seed);

            considerBest(candidate, best, config);

            ++done;
            publishProgress(config);
            progress.maybePrint(done, best, candidate.score, reportedRestart, iteration);
        }
    }

    return best;
}

SearchResult hillClimb(const SearchConfig& config) {
    validateConfig(config);

    const unsigned int seed = effectiveSeed(config.seed);
    std::mt19937 rng(seed);

    SearchResult best;
    best.seed = seed;

    const int totalIterations = config.iterations * config.restarts;
    ProgressPrinter progress(totalIterations, config.progressEvery, config.verbose);

    int done = 0;

    for (int restart = 1; restart <= config.restarts; ++restart) {
        const int reportedRestart = config.restartOffset + restart;
        SearchResult current = makeInitialCandidate(config, rng, seed, reportedRestart);

        considerBest(current, best, config);

        for (int iteration = 1; iteration <= config.iterations; ++iteration) {
            Grid candidateGrid = current.grid;

            auto [r, c] = randomMutableCell(config.n, rng);
            flipCell(candidateGrid, r, c);

            SearchResult candidate = evaluateGrid(std::move(candidateGrid),
                                                  config.requireReachable,
                                                  reportedRestart,
                                                  iteration,
                                                  seed);

            if (candidate.score >= current.score)
                current = candidate;

            considerBest(current, best, config);

            ++done;
            publishProgress(config);
            progress.maybePrint(done, best, current.score, reportedRestart, iteration);
        }
    }

    return best;
}

SearchResult simulatedAnnealing(const AnnealingConfig& config) {
    validateConfig(config);

    if (config.startTemperature <= 0.0 || config.endTemperature <= 0.0)
        throw std::invalid_argument("temperatures must be positive");

    const unsigned int seed = effectiveSeed(config.seed);
    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> real01(0.0, 1.0);

    SearchResult best;
    best.seed = seed;

    const int totalIterations = config.iterations * config.restarts;
    ProgressPrinter progress(totalIterations, config.progressEvery, config.verbose);

    int done = 0;

    for (int restart = 1; restart <= config.restarts; ++restart) {
        const int reportedRestart = config.restartOffset + restart;
        SearchResult current = makeInitialCandidate(config, rng, seed, reportedRestart);

        considerBest(current, best, config);

        for (int iteration = 1; iteration <= config.iterations; ++iteration) {
            Grid candidateGrid = current.grid;

            auto [r, c] = randomMutableCell(config.n, rng);
            flipCell(candidateGrid, r, c);

            SearchResult candidate = evaluateGrid(std::move(candidateGrid),
                                                  config.requireReachable,
                                                  reportedRestart,
                                                  iteration,
                                                  seed);

            const int delta = candidate.score - current.score;
            const double temperature = temperatureAt(config, iteration);

            bool accept = delta >= 0;
            if (!accept) {
                const double probability = std::exp(static_cast<double>(delta) / temperature);
                accept = real01(rng) < probability;
            }

            if (accept)
                current = candidate;

            considerBest(current, best, config);

            ++done;
            publishProgress(config);
            progress.maybePrint(done, best, current.score, reportedRestart, iteration);
        }
    }

    return best;
}

} // namespace spq
