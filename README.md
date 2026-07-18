# KhaosChess
This project was created as a diploma thesis for the completion of my Bachelor's degree in Computer Science.

Development continues beyond the thesis with a single goal: make the engine as strong as possible, one measured improvement at a time.

---

## Status

### Working
- **Board representation**: bitboards with magic sliding-piece attacks; full legal move generation, verified with a perft test suite against known positions
- **Search**: iterative-deepening negamax alpha-beta with quiescence search, principal variation search with aspiration windows, transposition table (Zobrist hashing), move ordering by killer moves, history heuristic, and static exchange evaluation (SEE), late move reductions, null-move pruning, forward pruning (reverse futility, late move pruning, futility), check extensions, draw detection (fifty-move rule and repetition), and time- or node-based stopping
- **Evaluation**: tapered middlegame/endgame scoring (material, pawn structure, king safety and more), specialized endgame evaluators keyed by material, and a KPK bitbase
- **UCI protocol**: plays complete games in GUIs (e.g. Arena) and match runners (e.g. fastchess)

### Roadmap
Each item gets validated with engine-vs-engine matches (see [Testing](#testing)) before it lands:

- Transposition table v2: depth-preferred replacement with aging, stored static eval, UCI `Hash` option (unlocks shelved qsearch-TT and IIR features)
- Countermove and continuation-history move ordering
- Smarter time management and UCI `stop` support
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

Strength changes are never eyeballed - they are measured with engine-vs-engine matches using [fastchess](https://github.com/Disservin/fastchess):

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

A healthy run ends with a roughly 50/50 score against an equal baseline and **zero disconnects or illegal moves** - the match runner doubles as an integration test that surfaces bugs perft never can (UCI parsing, endgame evaluation, time management).

### Measured progress

Each row was measured against the frozen baseline binary that preceded it; features land in batches, so several rows can ship in one release. Error bars are 95% confidence.

| Landed in | Change | Match conditions | Elo |
|-----------|--------|------------------|-----|
| 2.3.0 | Killer-move and history-heuristic ordering | 100 games, tc 8+0.08 | +168 ± 68 |
| 2.3.0 | Null-move pruning | 200 games, tc 8+0.08 | +93 ± 44 |
| 2.5.0 | Principal variation search | 280 games, tc 8+0.08 | +12 ± 34 |
| 2.5.0 | Late move reductions, reverse futility, late move pruning, futility pruning, SEE | 200 games, tc 8+0.08 | +87 ± 42 |
| 2.5.0 | Aspiration windows, check extensions | 200 games, 40k fixed nodes | +26 ± 41 |
| **2.5.0** | **Cumulative vs 2.3.0** | **500 games, tc 8+0.08** | **+81 ± 27** |
| 2.6.0 | Evaluation bug fixes: king shelter after castling, inverted king mobility, back-rank blockers, pin-aware mobility, semi-open files, x-ray batteries | 400 games, 20k fixed nodes | +53 ± 29 |
| 2.7.0 | Tempo bonus, piece-square tables (CPW-seeded, tapered), full connected-rooks bonus | 400 games, 20k fixed nodes | +59 ± 29 |
| 2.8.0 | Texel-tuned evaluation weights (829 parameters, coordinate descent over 725k zurichess positions) | 400 games, 20k fixed nodes | +291 ± 41 |

Fixed-node matches (`go nodes`) are used for changes where timing noise would drown the signal; they deliberately ignore speed costs, which is why the cumulative timed number runs below the sum of the parts.

---

## Evaluation tuning (Texel method)

The evaluation weights - material values, mobility bonuses, king-safety penalties, piece-square tables - are not hand-guessed. They are fitted with [Texel tuning](https://www.chessprogramming.org/Texel%27s_Tuning_Method): a static evaluation is secretly a prediction of who wins, so every weight is adjusted until the eval best predicts the results of thousands of real games.

The pipeline lives in `tools/`:

1. **Get labeled positions.** Either download a public dataset - the tuner natively reads zurichess-style EPD (`... c9 "1-0";`), Ethereal-style books (`<fen> [1.0]`) and plain `<fen>;<result>` lines - or extract your own from fastchess match PGNs with `tools/pgn_to_dataset.py` (requires `python-chess`):

   ```bash
   python3 tools/pgn_to_dataset.py 'path/to/*.pgn' > positions.txt
   ```

   Every finished game donates its quiet positions, each labeled with the game's final result. Positions in check, positions where the played move was a capture or promotion, the opening-book plies, and the final plies are all skipped - in real search the eval only ever scores quiet positions, so only those may teach it.

2. **Fit the weights** with the tuner (`tools/tuner.cpp`, built as `bin/tuner`; building it needs Intel TBB, e.g. `dnf install tbb-devel` - the engine itself stays dependency-free). It maps each eval score through a logistic curve to a win probability, measures the mean squared error against the actual results over the whole dataset in parallel (`std::execution::par`), and follows the analytic gradient of that error (Adam steps over cached feature vectors, re-extracted in a trust-region loop - see [tools/gradient-tuner.md](tools/gradient-tuner.md) for the maths):

   ```bash
   ./bin/tuner quiet-labeled.epd          # full run (hours; checkpoints every iteration)
   ./bin/tuner quiet-labeled.epd 10000    # quick smoke test
   ```

   The weights are exposed through a registry (`include/tune.h`, `src/tune.cpp`) - the same idea as Stockfish's `TUNE` macro: eval constants are mutable globals with compile-time defaults, so the optimizer can adjust them in memory without rebuilding the engine.

3. **Validate.** Tuned values are pasted back into `include/score.h` as the new defaults, and the rebuilt engine must beat the previous baseline in a fastchess match before anything lands (see [Measured progress](#measured-progress)). Fitting the data better is a proxy; winning games is the objective.

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
