# KhaosChess
This project was created as a diploma thesis for the completion of my Bachelor's degree in Computer Science.

---

## Build Instructions

### Requirements:
- CMake ≥ 3.10
- C++17-compatible compiler (GCC / Clang / MSVC)

---
### Build steps:

To manually build the project, you can follow these steps:

Firstly clone the repository:

```bash
# 1. Clone the repository
git clone https://github.com/FMI-OpenSource-Lab/chess-engine.git
cd chess-engine
```

Then create a build directory and navigate into it:
```bash
mkdir build
cd build
```

To configure and build the project in `debug` or `release` mode, you can use the following commands:
```bash
# Release mode (default)
cmake ..

# Debug mode
cmake -DCMAKE_BUILD_TYPE=Debug .. -DCMAKE_CXX_FLAGS_DEBUG="-DDEBUG_MODE"

# Build
cmake --build .
```

After building, when already in `build` folder, you can run the engine with:
```bash
./KhaosChess
```

Alternatively, if you are on Linux, you can use the provided `run.sh` script to automate these steps.
This script also supports an `install` option to install the binary on the root where it could be accessed from anywhere.
**Requires root privileges for installation.**

Clone the repository as mentioned above, then run in the **project** root:
```bash
source run.sh
```

After that a menu would appear prompting the user to choose between building in `debug`, `release` mode or doing an `incremental-build` with custom CMake flags.

---
### Visualization:

You can use KhaosChess with a graphical chess interface (GUI) like [Arena Chess GUI](https://www.playwitharena.de/)

#### How to integrate with Arena GUI (Windows example):

1. Open Arena GUI.
2. Go to **Engines → Install New Engine**.
3. Navigate to your compiled binary called `KhaosChess` (on Windows:`KhaosChess.exe`) and select it.
4. Arena will prompt you for engine settings (you can leave most as default).
5. The engine will now appear in your list and can be used to play, test or run engine-vs-engine matches.

**Note**: The engine accepts the **UCI (Universal Chess Interface)** protocol
