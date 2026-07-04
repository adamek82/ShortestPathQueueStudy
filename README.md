# ShortestPathQueueStudy

A small C++20 research project for studying the peak BFS queue size in
[LeetCode 1091 — Shortest Path in Binary Matrix](https://leetcode.com/problems/shortest-path-in-binary-matrix/).

The original problem asks for the shortest path from the top-left corner to the
bottom-right corner in an `n x n` binary grid, where:

- `0` means an open cell,
- `1` means a blocked cell,
- movement is allowed in 8 directions.

This project keeps the standard BFS solution, but instruments it to measure the
maximum number of elements stored in the BFS queue at any moment.

The motivation is to experimentally investigate whether the queue size behaves
like `O(n)` or can grow much larger for carefully constructed obstacle layouts.

## Features

- C++20 implementation.
- CMake build system.
- VS Code tasks and debug configuration.
- Windows/MSYS2 UCRT64 support.
- Linux/WSL support.
- Instrumented BFS for LeetCode 1091.
- Random grid search.
- Hill climbing search.
- Simulated annealing search.
- Built-in reporting of:
  - shortest path length,
  - whether the target is reachable,
  - visited cell count,
  - maximum BFS queue size,
  - `4n`,
  - `maxQ / n`,
  - `maxQ / n^2`.

## Project layout

```text
ShortestPathQueueStudy/
  .clang-format
  .gitignore
  CMakeLists.txt
  ShortestPathQueueStudy.code-workspace
  README.md
  .vscode/
    c_cpp_properties.json
    launch.json
    settings.json
    tasks.json
  include/
    bfs.hpp
    cli.hpp
    grid.hpp
    reference_grids.hpp
    search.hpp
  src/
    bfs.cpp
    cli.cpp
    grid.cpp
    main.cpp
    reference_grids.cpp
    search.cpp
```

## Build

### Windows / MSYS2 UCRT64

Install MSYS2 and the UCRT64 toolchain.

Recommended packages:

```bash
pacman -S --needed \
    mingw-w64-ucrt-x86_64-gcc \
    mingw-w64-ucrt-x86_64-gdb \
    mingw-w64-ucrt-x86_64-cmake \
    mingw-w64-ucrt-x86_64-make \
    mingw-w64-ucrt-x86_64-clang-tools-extra
```

Configure and build from PowerShell:

```powershell
C:/msys64/ucrt64/bin/cmake.exe `
    -S . `
    -B build/debug `
    -G "MinGW Makefiles" `
    -DCMAKE_BUILD_TYPE=Debug `
    -DCMAKE_CXX_COMPILER=C:/msys64/ucrt64/bin/g++.exe

C:/msys64/ucrt64/bin/cmake.exe --build build/debug
```

Run:

```powershell
$env:PATH = "C:\msys64\ucrt64\bin;" + $env:PATH
.\build\debug\ShortestPathQueueStudy.exe --mode ref
```

The temporary `PATH` update is needed when running the executable from a normal
PowerShell, because the MSYS2/UCRT64-built executable needs runtime DLLs from:

```text
C:\msys64\ucrt64\bin
```

VS Code tasks and launch configurations set this automatically.

### Linux / WSL

Install the usual build tools:

```bash
sudo apt update
sudo apt install build-essential gdb cmake clang-format
```

Configure and build:

```bash
cmake -S . -B build/debug -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=g++
cmake --build build/debug
```

Run:

```bash
./build/debug/ShortestPathQueueStudy --mode ref
```

## VS Code usage

Open the workspace file:

```text
ShortestPathQueueStudy.code-workspace
```

Useful tasks:

- `Build Debug`
- `Run Debug`
- `Build Release`
- `Clean`

The default build task is `Build Debug`, so `Ctrl+Shift+B` should build the debug version.

The debugger configuration uses GDB:

- Windows: `C:/msys64/ucrt64/bin/gdb.exe`
- Linux/WSL: `/usr/bin/gdb`

If IntelliSense selects the wrong compiler, run:

```text
Ctrl+Shift+P
C/C++: Select a Configuration
```

Then select:

```text
MSYS2 UCRT64
```

on Windows, or:

```text
Linux GCC
```

on Linux/WSL.

## Usage

Show help:

```powershell
.\build\debug\ShortestPathQueueStudy.exe --help
```

Run the built-in reference grid:

```powershell
.\build\debug\ShortestPathQueueStudy.exe --mode ref
```

Run random search:

```powershell
.\build\debug\ShortestPathQueueStudy.exe --mode random --n 20 --iterations 100000 --seed 123
```

Run hill climbing:

```powershell
.\build\debug\ShortestPathQueueStudy.exe --mode hill --n 20 --iterations 200000 --restarts 10 --seed 123
```

Run simulated annealing:

```powershell
.\build\debug\ShortestPathQueueStudy.exe --mode anneal --n 20 --iterations 300000 --restarts 10 --seed 123
```

Allow unreachable grids:

```powershell
.\build\debug\ShortestPathQueueStudy.exe --mode anneal --n 20 --iterations 300000 --restarts 10 --seed 123 --allow-unreachable
```

On Linux/WSL, replace:

```text
.\build\debug\ShortestPathQueueStudy.exe
```

with:

```text
./build/debug/ShortestPathQueueStudy
```

## Command-line options

```text
--mode ref|random|hill|anneal
```

Selects the mode.

- `ref` runs a built-in reference grid.
- `random` generates independent random grids.
- `hill` uses hill climbing.
- `anneal` uses simulated annealing.

```text
--n N
```

Grid size. Default: `20`.

```text
--iterations N
```

Number of iterations per restart. Default: `100000`.

```text
--restarts N
```

Number of independent restarts. Default: `10`.

```text
--density P
```

Initial obstacle probability for generated grids. Default: `0.35`.

```text
--seed N
```

Random seed. If omitted or set to `0`, the program uses `std::random_device`.

```text
--progress-every N
```

Progress print interval. Default: `10000`.

```text
--require-reachable true|false
```

Controls whether unreachable grids are accepted as valid candidates. Default: `true`.

```text
--allow-unreachable
```

Shortcut for:

```text
--require-reachable false
```

```text
--start-temp T
```

Starting temperature for simulated annealing. Default: `5.0`.

```text
--end-temp T
```

Final temperature for simulated annealing. Default: `0.01`.

```text
--quiet
```

Disables progress logs.

## Output interpretation

Example progress line:

```text
restart=2 iter=50000 done=150000/1000000 current=28 best=47 bestPath=21 elapsed=11.5s eta=1m 5s
```

Meaning:

```text
restart=2
```

The current restart number.

```text
iter=50000
```

Iteration number within the current restart.

```text
done=150000/1000000
```

Global progress across all restarts.

```text
current=28
```

Score of the currently evaluated grid.

The score is currently defined as:

```text
score = max BFS queue size
```

If `--require-reachable true` is enabled and the target is not reachable, the score is:

```text
-1
```

```text
best=47
```

Best score found so far during the whole run.

```text
bestPath=21
```

Shortest path length of the best grid found so far.

```text
elapsed=11.5s
```

Elapsed runtime.

```text
eta=1m 5s
```

Estimated time remaining.

## Algorithms

### Instrumented BFS

The BFS is based on the standard solution for LeetCode 1091.

It uses the input grid itself as the visited/distance array:

```text
0  = open and unvisited
1  = originally blocked, or distance 1 for the start cell
>1 = visited cell with BFS distance from start
```

The BFS queue stores cells as `(row, column)` pairs.

The important measured value is:

```text
maxQueueSize
```

which is updated after every enqueue operation.

### Random search

Random search generates independent random grids and keeps the one with the largest observed
`maxQueueSize`.

It is a useful baseline, but it usually finds weaker examples than hill climbing or simulated annealing.

### Hill climbing

Hill climbing starts from a random grid and repeatedly flips one random non-endpoint cell.

If the mutation improves the score, the change is kept. Otherwise, it is rejected.

This is simple and often effective, but it can get stuck in local maxima.

### Simulated annealing

Simulated annealing is similar to hill climbing, but it sometimes accepts worse moves.

At the beginning, the temperature is high, so worse moves are accepted more often. Later, the temperature
decreases, and the search becomes more conservative.

This helps the search escape local maxima.

## Research notes

For an empty `n x n` grid, the BFS frontier is geometrically simple and its size is roughly linear in `n`.

With obstacles, the BFS frontier can become fragmented. The queue may contain many discovered but not
yet processed cells from the current and next BFS layers.

This project is meant to experimentally search for grids where the queue becomes large.

A useful sanity check is comparing:

```text
maxQueueSize
```

against:

```text
4n
```

and:

```text
maxQueueSize / n
maxQueueSize / n^2
```

For example, for `n = 20`, `4n = 80`. Any grid with `maxQueueSize > 80` is already a counterexample to
the simple hypothesis:

```text
max queue size <= 4n
```

Larger grids are useful for checking whether the growth looks closer to linear or quadratic.

## Formatting

Linux / WSL:

```bash
clang-format -i src/*.cpp include/*.hpp
```

Windows / MSYS2 UCRT64:

```powershell
C:/msys64/ucrt64/bin/clang-format.exe -i src/*.cpp include/*.hpp
```

## Possible future work

- Add multi-threaded search.
- Add CSV output for comparing runs.
- Add a mode for saving best grids to files.
- Add visualization of BFS distance layers.
- Add visualization of queue size over time.
- Add more mutation operators than single-cell flips.
- Add support for rectangular grids.
- Compare reachable-only and unreachable-allowed searches.
