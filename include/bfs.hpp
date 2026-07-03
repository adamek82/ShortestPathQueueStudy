#pragma once

#include "grid.hpp"

namespace spq {

struct BfsStats {
    int shortestPath{-1};
    int maxQueueSize{0};
    int visitedCells{0};
    bool reachedTarget{false};
};

BfsStats analyzeShortestPathBinaryMatrix(Grid grid);

} // namespace spq
