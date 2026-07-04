#include "bfs.hpp"
#include "cli.hpp"
#include "grid.hpp"
#include "reference_grids.hpp"
#include "search.hpp"

#include <exception>
#include <iomanip>
#include <iostream>
#include <string>

namespace {

void printStats(const spq::Grid& grid, const spq::BfsStats& stats) {
    const int n = spq::gridSize(grid);

    std::cout << "n: " << n << '\n';
    std::cout << "shortest path: " << stats.shortestPath << '\n';
    std::cout << "reached target: " << (stats.reachedTarget ? "yes" : "no") << '\n';
    std::cout << "visited cells: " << stats.visitedCells << '\n';
    std::cout << "max queue size: " << stats.maxQueueSize << '\n';
    std::cout << "4n: " << 4 * n << '\n';

    if (n > 0) {
        std::cout << "maxQ / n: " << std::fixed << std::setprecision(3)
                  << static_cast<double>(stats.maxQueueSize) / n << '\n';
        std::cout << "maxQ / n^2: " << std::fixed << std::setprecision(3)
                  << static_cast<double>(stats.maxQueueSize) / (n * n) << '\n';
    }
}

void printResult(const spq::SearchResult& result) {
    std::cout << "\n=== Best result ===\n";
    std::cout << "seed: " << result.seed << '\n';
    std::cout << "restart: " << result.restart << '\n';
    std::cout << "iteration: " << result.iteration << '\n';
    std::cout << "score: " << result.score << '\n';

    printStats(result.grid, result.stats);

    std::cout << "\nGrid view:\n";
    spq::printGrid(result.grid, std::cout);

    std::cout << "\nC++ initializer:\n";
    spq::printGridAsCppInitializer(result.grid, std::cout, "grid");
}

void runReference() {
    spq::Grid grid = spq::referenceGrid20x20MaxQueue126();
    spq::BfsStats stats = spq::analyzeShortestPathBinaryMatrix(grid);

    std::cout << "=== Reference 20x20 grid, max queue 126 ===\n";
    printStats(grid, stats);

    std::cout << "\nGrid view:\n";
    spq::printGrid(grid, std::cout);

    std::cout << "\nC++ initializer:\n";
    spq::printGridAsCppInitializer(grid, std::cout, "grid");
}

} // namespace

int main(int argc, char** argv) {
    try {
        const spq::ProgramOptions options = spq::parseArgs(argc, argv);

        if (options.help) {
            spq::printUsage(std::cout, argv[0]);
            return 0;
        }

        if (options.mode == "ref") {
            runReference();
            return 0;
        }

        if (options.mode == "random") {
            spq::SearchResult result = spq::randomSearch(spq::makeSearchConfig(options));
            printResult(result);
            return 0;
        }

        if (options.mode == "hill") {
            spq::SearchResult result = spq::hillClimb(spq::makeSearchConfig(options));
            printResult(result);
            return 0;
        }

        if (options.mode == "anneal") {
            spq::SearchResult result =
                spq::simulatedAnnealing(spq::makeAnnealingConfig(options));
            printResult(result);
            return 0;
        }

        std::cerr << "Unknown mode: " << options.mode << "\n\n";
        spq::printUsage(std::cerr, argv[0]);
        return 1;
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << '\n';
        return 1;
    }
}
