# KhaosChess
This project was created as a diploma thesis for the completion of my Bachelor's degree in Computer Science.

Development continues beyond the thesis with a single goal: make the engine as strong as possible, one measured improvement at a time.

---

## Status

### Working
- **Board representation**: bitboards with magic sliding-piece attacks; full legal move generation, verified with a perft test suite against known positions
- **Search**: iterative-deepening negamax alpha-beta with quiescence search, transposition table (Zobrist hashing), draw detection (fifty-move rule and threefold repetition), and time-based stopping
- **Evaluation**: tapered middlegame/endgame scoring (material, pawn structure, king safety and more), specialized endgame evaluators keyed by material, and a KPK bitbase
- **UCI protocol**: plays complete games in GUIs (e.g. Arena) and match runners (e.g. fastchess)

### Roadmap
Each item gets validated with engine-vs-engine matches (see [Testing](#testing)) before it lands:

- Killer moves + history heuristic move ordering
- Null-move pruning and late move reductions
- Principal variation search + aspiration windows
- Static exchange evaluation (SEE)
- Smarter time management and UCI `stop` support
- Texel tuning of the evaluation weights
- Long term: Lazy SMP, NNUE evaluation

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
cmake -DCMAKE_BUILD_TYPE=Debug ..

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

## Testing

### Unit tests

A GoogleTest suite lives in `tests/` and covers position handling (FEN round-trips, do/undo), perft move-generation ladders, bitboard attack generation, and draw detection. The test binaries are built together with the engine and placed in `bin/tests/`:

```bash
./bin/tests/position_tests
./bin/tests/perft_tests
./bin/tests/bitboard_tests
./bin/tests/draw_tests
```

### Engine matches (fastchess)

Strength changes are never eyeballed — they are measured with engine-vs-engine matches using [fastchess](https://github.com/Disservin/fastchess):

- A **frozen baseline binary** (`KhaosChess-base`) is kept as the reference opponent. Harness and protocol fixes get folded into the baseline; strength changes never do.
- Games start from a book of varied openings (e.g. an 8-move opening book), each opening played twice with colors swapped.
- With ~100 games the error bars are still around ±45 Elo, so small improvements need many hundreds of games (or SPRT) before they can be trusted.

Example sanity match:

```bash
fastchess \
  -engine cmd=./bin/KhaosChess name=khaos \
  -engine cmd=./bin/KhaosChess-base name=khaos-base \
  -each tc=8+0.08 \
  -openings file=8moves_v3.pgn format=pgn order=random \
  -rounds 50 -repeat -concurrency 2 -recover \
  -pgnout file=match.pgn
```

A healthy run ends with a roughly 50/50 score against an equal baseline and **zero disconnects or illegal moves** — the match runner doubles as an integration test that surfaces bugs perft never can (UCI parsing, endgame evaluation, time management).

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
