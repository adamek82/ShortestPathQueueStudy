# CppCMakeVSCodeTemplate

A minimal C++20 project template using CMake, VS Code, MSYS2 UCRT64 on Windows, and GCC on Linux/WSL.

The goal of this repository is to provide a small, clean starting point for C++ projects with:

- CMake build configuration
- Debug and Release builds
- VS Code build/debug tasks
- MSYS2 UCRT64 support on Windows
- Linux/WSL support
- GDB debugging
- `.clang-format`
- simple `include/` and `src/` layout

## Project layout

```text
CppCMakeVSCodeTemplate/
  .clang-format
  .gitignore
  CMakeLists.txt
  CppCMakeVSCodeTemplate.code-workspace
  README.md
  .vscode/
    c_cpp_properties.json
    launch.json
    settings.json
    tasks.json
  include/
    app.hpp
  src/
    main.cpp
```

## Prerequisites

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

Expected paths:

```text
C:/msys64/ucrt64/bin/g++.exe
C:/msys64/ucrt64/bin/gdb.exe
C:/msys64/ucrt64/bin/cmake.exe
```

### Linux / WSL

Install the usual build tools:

```bash
sudo apt update
sudo apt install build-essential gdb cmake clang-format
```

## Build

### Windows / PowerShell

```powershell
$env:PATH = "C:\msys64\ucrt64\bin;C:\msys64\usr\bin;" + $env:PATH

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
.\build\debug\CppCMakeVSCodeTemplate.exe
```

### Linux / WSL

```bash
cmake -S . -B build/debug -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=g++
cmake --build build/debug
./build/debug/CppCMakeVSCodeTemplate
```

## VS Code usage

Open:

```text
CppCMakeVSCodeTemplate.code-workspace
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

## Rename template

The main project token is:

```text
CppCMakeVSCodeTemplate
```

To create a new project, replace it with your project name in:

- `CMakeLists.txt`
- `CppCMakeVSCodeTemplate.code-workspace`
- `.vscode/launch.json`
- `.vscode/tasks.json`
- `README.md`
- `src/main.cpp`

Also rename:

```text
CppCMakeVSCodeTemplate.code-workspace
```

Example PowerShell replacement:

```powershell
$old = "CppCMakeVSCodeTemplate"
$new = "MyNewProject"

Get-ChildItem -Recurse -File |
    Where-Object {
        $_.FullName -notmatch '\\\.git\\' -and
        $_.FullName -notmatch '\\build\\'
    } |
    ForEach-Object {
        (Get-Content $_.FullName) -replace $old, $new |
            Set-Content $_.FullName
    }

Rename-Item "CppCMakeVSCodeTemplate.code-workspace" "$new.code-workspace"
```

## Format

Linux / WSL:

```bash
clang-format -i src/*.cpp include/*.hpp
```

Windows / MSYS2 UCRT64:

```powershell
C:/msys64/ucrt64/bin/clang-format.exe -i src/*.cpp include/*.hpp
```

## Notes

This template intentionally keeps the project small. It is meant to be a clean starting point, not a framework.
