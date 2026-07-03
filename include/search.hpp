#pragma once

#include "bfs.hpp"
#include "grid.hpp"

namespace spq {

struct SearchConfig {
    int n{20};
    int iterations{100000};
    int restarts{10};
    double initialBlockProbability{0.35};
    bool requireReachable{true};
    int progressEvery{10000};
    unsigned int seed{0};
    bool verbose{true};
};

struct AnnealingConfig : SearchConfig {
    double startTemperature{5.0};
    double endTemperature{0.01};
};

struct SearchResult {
    Grid grid;
    BfsStats stats;
    int score{-1};
    int restart{0};
    int iteration{0};
    unsigned int seed{0};
};

SearchResult randomSearch(const SearchConfig& config);
SearchResult hillClimb(const SearchConfig& config);
SearchResult simulatedAnnealing(const AnnealingConfig& config);

} // namespace spq
