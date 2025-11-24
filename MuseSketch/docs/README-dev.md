# MuseSketch Developer Guide

## Prerequisites

- **CMake**: 3.16 or higher
- **Qt**: 6.x (Core, Gui, Qml, Quick)
- **Compiler**: C++17 compatible (Clang, GCC, MSVC)

## Setup

### macOS

```bash
brew install cmake qt
```

## Building

### Desktop (Debug)

```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build .
```

### Running

```bash
open app/musesketch.app
```

## Project Structure

- `app/`: Main application code (C++ & QML)
- `core/`: Core logic and data models (to be implemented)
- `third_party/`: External dependencies (MuseScore core, etc.)
- `cmake/`: CMake helper modules
- `docs/`: Documentation
- `scripts/`: Build/helper scripts
- `tests/`: Unit tests

## Roadmap

See [ROADMAP.md](ROADMAP.md) for the development plan.
