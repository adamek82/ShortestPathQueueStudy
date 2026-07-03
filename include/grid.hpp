#pragma once

#include <iosfwd>
#include <random>
#include <string>
#include <utility>
#include <vector>

namespace spq {

using Grid = std::vector<std::vector<int>>;

int gridSize(const Grid& grid);
bool isSquareGrid(const Grid& grid);

void forceEndpointsOpen(Grid& grid);
Grid makeRandomGrid(int n, double blockProbability, std::mt19937& rng);

std::pair<int, int> randomMutableCell(int n, std::mt19937& rng);
void flipCell(Grid& grid, int r, int c);

void printGrid(const Grid& grid, std::ostream& out);
void printGrid01(const Grid& grid, std::ostream& out);
void printGridAsCppInitializer(const Grid& grid, std::ostream& out,
                               const std::string& variableName = "grid");

} // namespace spq
