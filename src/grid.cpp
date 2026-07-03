#include "grid.hpp"

#include <iomanip>
#include <iostream>
#include <stdexcept>

namespace spq {

int gridSize(const Grid& grid) {
    return static_cast<int>(grid.size());
}

bool isSquareGrid(const Grid& grid) {
    if (grid.empty())
        return false;

    const auto n = grid.size();
    for (const auto& row : grid) {
        if (row.size() != n)
            return false;
    }

    return true;
}

void forceEndpointsOpen(Grid& grid) {
    if (grid.empty())
        return;

    const int n = gridSize(grid);
    grid[0][0] = 0;
    grid[n - 1][n - 1] = 0;
}

Grid makeRandomGrid(int n, double blockProbability, std::mt19937& rng) {
    if (n <= 0)
        throw std::invalid_argument("Grid size must be positive");

    if (blockProbability < 0.0 || blockProbability > 1.0)
        throw std::invalid_argument("Block probability must be in [0, 1]");

    std::bernoulli_distribution blocked(blockProbability);

    Grid grid(n, std::vector<int>(n, 0));
    for (auto& row : grid) {
        for (int& cell : row) {
            cell = blocked(rng) ? 1 : 0;
        }
    }

    forceEndpointsOpen(grid);
    return grid;
}

std::pair<int, int> randomMutableCell(int n, std::mt19937& rng) {
    if (n <= 1)
        throw std::invalid_argument("Need n >= 2 to mutate non-endpoint cells");

    std::uniform_int_distribution<int> dist(0, n - 1);

    while (true) {
        int r = dist(rng);
        int c = dist(rng);

        const bool start = (r == 0 && c == 0);
        const bool target = (r == n - 1 && c == n - 1);

        if (!start && !target)
            return {r, c};
    }
}

void flipCell(Grid& grid, int r, int c) {
    grid[r][c] = (grid[r][c] == 0) ? 1 : 0;
}

void printGrid(const Grid& grid, std::ostream& out) {
    const int n = gridSize(grid);

    for (int r = 0; r < n; ++r) {
        for (int c = 0; c < n; ++c) {
            if (r == 0 && c == 0) {
                out << 'S';
            } else if (r == n - 1 && c == n - 1) {
                out << 'T';
            } else {
                out << (grid[r][c] == 0 ? '.' : '#');
            }

            if (c + 1 < n)
                out << ' ';
        }
        out << '\n';
    }
}

void printGrid01(const Grid& grid, std::ostream& out) {
    const int n = gridSize(grid);

    for (int r = 0; r < n; ++r) {
        for (int c = 0; c < n; ++c) {
            out << grid[r][c];
            if (c + 1 < n)
                out << ' ';
        }
        out << '\n';
    }
}

void printGridAsCppInitializer(const Grid& grid, std::ostream& out,
                               const std::string& variableName) {
    const int n = gridSize(grid);

    out << "Grid " << variableName << " = {\n";

    for (int r = 0; r < n; ++r) {
        out << "    {";
        for (int c = 0; c < n; ++c) {
            out << grid[r][c];
            if (c + 1 < n)
                out << ',';
        }
        out << "}";

        if (r + 1 < n)
            out << ',';

        out << '\n';
    }

    out << "};\n";
}

} // namespace spq
