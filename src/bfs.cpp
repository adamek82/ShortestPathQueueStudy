#include "bfs.hpp"

#include <algorithm>
#include <array>
#include <queue>
#include <utility>

namespace spq {

BfsStats analyzeShortestPathBinaryMatrix(Grid grid) {
    BfsStats stats;

    if (!isSquareGrid(grid))
        return stats;

    const int n = gridSize(grid);

    if (n == 0 || grid[0][0] != 0 || grid[n - 1][n - 1] != 0)
        return stats;

    static constexpr std::array<std::pair<int, int>, 8> kDir{{
        {-1, -1},
        {-1, 0},
        {-1, 1},
        {0, -1},
        {0, 1},
        {1, -1},
        {1, 0},
        {1, 1},
    }};

    std::queue<std::pair<int, int>> q;
    q.emplace(0, 0);
    stats.maxQueueSize = 1;

    grid[0][0] = 1;

    while (!q.empty()) {
        auto [r, c] = q.front();
        q.pop();

        ++stats.visitedCells;

        const int dist = grid[r][c];

        if (r == n - 1 && c == n - 1) {
            stats.shortestPath = dist;
            stats.reachedTarget = true;
            return stats;
        }

        for (auto [dr, dc] : kDir) {
            const int nr = r + dr;
            const int nc = c + dc;

            if (nr < 0 || nc < 0 || nr >= n || nc >= n)
                continue;

            if (grid[nr][nc] != 0)
                continue;

            grid[nr][nc] = dist + 1;
            q.emplace(nr, nc);

            stats.maxQueueSize =
                std::max(stats.maxQueueSize, static_cast<int>(q.size()));
        }
    }

    return stats;
}

} // namespace spq
