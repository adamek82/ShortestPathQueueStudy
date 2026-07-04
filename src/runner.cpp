#include "runner.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cctype>
#include <cstdint>
#include <future>
#include <iomanip>
#include <iostream>
#include <limits>
#include <mutex>
#include <random>
#include <sstream>
#include <stdexcept>
#include <thread>
#include <vector>

namespace spq {
namespace {

std::string lower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
    return value;
}

unsigned int resolveBaseSeed(unsigned int seed) {
    if (seed != 0)
        return seed;

    return std::random_device{}();
}

unsigned int mixSeed(unsigned int baseSeed, int workerIndex) {
    std::uint32_t x = baseSeed;
    x += 0x9e3779b9u + static_cast<std::uint32_t>(workerIndex) * 0x85ebca6bu;

    x ^= x >> 16;
    x *= 0x7feb352du;
    x ^= x >> 15;
    x *= 0x846ca68bu;
    x ^= x >> 16;

    if (x == 0)
        x = static_cast<std::uint32_t>(workerIndex + 1);

    return x;
}

long long totalBudget(const AnnealingConfig& config) {
    return static_cast<long long>(config.iterations) * config.restarts;
}

void validateRunnerConfig(const AnnealingConfig& config) {
    if (config.iterations <= 0)
        throw std::invalid_argument("iterations must be positive");

    if (config.restarts <= 0)
        throw std::invalid_argument("restarts must be positive");
}

std::string formatSeconds(double seconds) {
    std::ostringstream os;

    if (seconds < 60.0) {
        os << std::fixed << std::setprecision(1) << seconds << "s";
        return os.str();
    }

    const int minutes = static_cast<int>(seconds / 60.0);
    const int restSeconds = static_cast<int>(seconds) % 60;

    os << minutes << "m " << restSeconds << "s";
    return os.str();
}

void printParallelProgress(long long done, long long total, const SearchResult& best,
                           const std::chrono::steady_clock::time_point& start) {
    const auto now = std::chrono::steady_clock::now();
    const double elapsed = std::chrono::duration<double>(now - start).count();

    const double ratio = done > 0 ? static_cast<double>(done) / static_cast<double>(total) : 0.0;
    const double totalEstimate = ratio > 0.0 ? elapsed / ratio : 0.0;
    const double eta = totalEstimate > elapsed ? totalEstimate - elapsed : 0.0;

    std::cout << "parallel done=" << done << "/" << total
              << " best=" << best.score
              << " bestPath=" << best.stats.shortestPath
              << " elapsed=" << formatSeconds(elapsed)
              << " eta=" << formatSeconds(eta)
              << '\n';
}

std::vector<AnnealingConfig> makeWorkerConfigs(SearchMode mode, const AnnealingConfig& config,
                                               int requestedThreadCount,
                                               unsigned int baseSeed,
                                               std::atomic<long long>& done,
                                               std::mutex& bestMutex,
                                               SearchResult& liveBest) {
    const long long total = totalBudget(config);

    if (total <= 0)
        throw std::invalid_argument("total search budget must be positive");

    int resolvedThreads = resolveThreadCount(requestedThreadCount);

    std::vector<AnnealingConfig> workers;

    auto progressCallback = [&done](int delta) {
        done.fetch_add(delta, std::memory_order_relaxed);
    };

    auto bestCallback = [&bestMutex, &liveBest](const SearchResult& candidate) {
        std::lock_guard<std::mutex> lock(bestMutex);
        if (isBetterSearchResult(candidate, liveBest))
            liveBest = candidate;
    };

    if (mode == SearchMode::Random) {
        const int activeThreads = static_cast<int>(
            std::min<long long>(resolvedThreads, total));

        workers.reserve(activeThreads);

        const long long baseIterations = total / activeThreads;
        const long long remainder = total % activeThreads;

        for (int worker = 0; worker < activeThreads; ++worker) {
            const long long assignedIterations =
                baseIterations + (worker < remainder ? 1 : 0);

            if (assignedIterations > std::numeric_limits<int>::max())
                throw std::overflow_error("Too many iterations assigned to one worker");

            AnnealingConfig workerConfig = config;
            workerConfig.restarts = 1;
            workerConfig.iterations = static_cast<int>(assignedIterations);
            workerConfig.restartOffset = worker;
            workerConfig.seed = mixSeed(baseSeed, worker);
            workerConfig.verbose = false;
            workerConfig.progressCallback = progressCallback;
            workerConfig.bestCallback = bestCallback;

            workers.push_back(std::move(workerConfig));
        }

        return workers;
    }

    const int activeThreads = std::min(resolvedThreads, config.restarts);
    workers.reserve(activeThreads);

    const int baseRestarts = config.restarts / activeThreads;
    const int remainder = config.restarts % activeThreads;

    int restartOffset = 0;

    for (int worker = 0; worker < activeThreads; ++worker) {
        const int assignedRestarts = baseRestarts + (worker < remainder ? 1 : 0);

        AnnealingConfig workerConfig = config;
        workerConfig.restarts = assignedRestarts;
        workerConfig.restartOffset = restartOffset;
        workerConfig.seed = mixSeed(baseSeed, worker);
        workerConfig.verbose = false;
        workerConfig.progressCallback = progressCallback;
        workerConfig.bestCallback = bestCallback;

        workers.push_back(std::move(workerConfig));
        restartOffset += assignedRestarts;
    }

    return workers;
}

bool allReady(std::vector<std::future<SearchResult>>& futures) {
    using namespace std::chrono_literals;

    for (auto& future : futures) {
        if (future.wait_for(0ms) != std::future_status::ready)
            return false;
    }

    return true;
}

} // namespace

SearchMode parseSearchMode(const std::string& mode) {
    const std::string m = lower(mode);

    if (m == "random")
        return SearchMode::Random;

    if (m == "hill")
        return SearchMode::Hill;

    if (m == "anneal")
        return SearchMode::Anneal;

    throw std::runtime_error("Unknown search mode: " + mode);
}

const char* searchModeName(SearchMode mode) {
    switch (mode) {
    case SearchMode::Random:
        return "random";
    case SearchMode::Hill:
        return "hill";
    case SearchMode::Anneal:
        return "anneal";
    }

    return "unknown";
}

int detectHardwareConcurrency() {
    const unsigned int detected = std::thread::hardware_concurrency();
    if (detected == 0)
        return 1;

    return static_cast<int>(detected);
}

int resolveThreadCount(int requestedThreads) {
    if (requestedThreads < 0)
        throw std::invalid_argument("threads must be non-negative");

    if (requestedThreads == 0)
        return detectHardwareConcurrency();

    return requestedThreads;
}

SearchResult runSearchWorker(SearchMode mode, AnnealingConfig config) {
    switch (mode) {
    case SearchMode::Random:
        return randomSearch(config);
    case SearchMode::Hill:
        return hillClimb(config);
    case SearchMode::Anneal:
        return simulatedAnnealing(config);
    }

    throw std::runtime_error("Unsupported search mode");
}

SearchResult runParallel(SearchMode mode, AnnealingConfig config, int requestedThreads) {
    validateRunnerConfig(config);

    const int resolvedThreads = resolveThreadCount(requestedThreads);

    if (resolvedThreads == 1)
        return runSearchWorker(mode, std::move(config));

    const unsigned int baseSeed = resolveBaseSeed(config.seed);
    const long long total = totalBudget(config);

    std::atomic<long long> done{0};
    std::mutex bestMutex;
    SearchResult liveBest;
    liveBest.seed = baseSeed;

    std::vector<AnnealingConfig> workerConfigs = makeWorkerConfigs(
        mode, config, resolvedThreads, baseSeed, done, bestMutex, liveBest);

    const auto start = std::chrono::steady_clock::now();

    if (config.verbose) {
        std::cout << "parallel mode=" << searchModeName(mode)
                  << " threads=" << workerConfigs.size()
                  << " requestedThreads=" << requestedThreads
                  << " hardwareConcurrency=" << detectHardwareConcurrency()
                  << " total=" << total
                  << " baseSeed=" << baseSeed
                  << '\n';
    }

    std::vector<std::future<SearchResult>> futures;
    futures.reserve(workerConfigs.size());

    for (auto& workerConfig : workerConfigs) {
        futures.push_back(std::async(std::launch::async, [mode, workerConfig]() mutable {
            return runSearchWorker(mode, std::move(workerConfig));
        }));
    }

    long long lastPrintedDone = 0;

    while (!allReady(futures)) {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(1s);

        if (!config.verbose)
            continue;

        const long long doneNow = done.load(std::memory_order_relaxed);

        const bool progressIntervalReached =
            config.progressEvery <= 0 ||
            doneNow - lastPrintedDone >= config.progressEvery;

        if (!progressIntervalReached)
            continue;

        SearchResult bestCopy;
        {
            std::lock_guard<std::mutex> lock(bestMutex);
            bestCopy = liveBest;
        }

        printParallelProgress(doneNow, total, bestCopy, start);
        lastPrintedDone = doneNow;
    }

    SearchResult best;
    best.seed = baseSeed;

    for (auto& future : futures) {
        SearchResult workerResult = future.get();

        if (isBetterSearchResult(workerResult, best))
            best = std::move(workerResult);
    }

    if (config.verbose) {
        SearchResult bestCopy;
        {
            std::lock_guard<std::mutex> lock(bestMutex);
            bestCopy = liveBest;
        }

        const long long doneNow = done.load(std::memory_order_relaxed);
        printParallelProgress(doneNow, total, bestCopy, start);
    }

    return best;
}

} // namespace spq
