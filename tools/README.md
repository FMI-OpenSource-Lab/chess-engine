# Tools

Offline tooling for tuning the evaluation. None of this code runs inside the
engine - the executables here link the engine's core as a library and use its
evaluation function directly. The big picture is in the main README's
[Evaluation tuning](../README.md#evaluation-tuning-texel-method) section;
this file is the practical reference.

The pipeline:

```
games (PGN)              --> pgn_to_dataset.py --> labeled positions --> tuner
positions (EPD, books)   ------------------------------------------------^
```

---

## pgn_to_dataset.py

Turns match PGNs into labeled training positions. Requires `python-chess`.

```bash
python3 tools/pgn_to_dataset.py 'path/to/*.pgn' > positions.txt
```

Output: one `<fen>;<result>` line per position, result from White's
perspective (`1.0` / `0.5` / `0.0`).

Only *quiet* positions from *finished* games are kept, because in real search
the evaluation only ever scores quiet positions (quiescence search resolves
the tactics first). Skipped: games without a result header, the first 16
plies (opening book - correlated across games), the last 6 plies (the result
is already decided), positions in check, positions where the played move was
a capture or promotion, and duplicates.

## tuner.cpp (bin/tuner)

Fits every evaluation weight to game results with
[Texel tuning](https://www.chessprogramming.org/Texel%27s_Tuning_Method).
Built together with the engine; the target needs Intel TBB
(`dnf install tbb-devel` / `apt install libtbb-dev`) because the error
computation runs parallel via `std::execution::par`.

```bash
./bin/tuner <positions-file> [max-positions] [max-outer] [trust] [resume-file]

./bin/tuner quiet-labeled.epd                              # full run (15 outers, trust 30)
./bin/tuner quiet-labeled.epd 10000                        # quick smoke test
./bin/tuner quiet-labeled.epd 0 15 30 gradient_params.txt  # resume a previous run
```

Accepted input formats, auto-detected per line (they can be mixed in one
file):

| format | example line |
|---|---|
| zurichess-style EPD | `rnbq.../... w KQkq - c9 "1-0";` |
| Ethereal-style book | `rnbq.../... w KQkq - 0 1 [1.0]` |
| plain | `rnbq.../... w KQkq - 0 1;1.0` |

### How it works

1. Every position is evaluated with the engine's real `Scorer<SC_ALL>` and
   mapped through a logistic curve to an expected score in 0..1:
   `1 / (1 + 10^(-K*eval/400))`.
2. The **error** is the mean squared difference between that prediction and
   the game's actual result, over the whole dataset.
3. **K** - the exchange rate between eval units and expected score - is
   fitted first by a coarse-then-fine grid scan (that is the
   `K = ..., initial error = ...` startup line), then frozen so that all
   later improvement must come from the weights.
4. **Gradient descent (Adam)**: the eval is linear in each of the ~830
   registered weights (see `include/tune.h`), so per-position coefficient
   vectors are extracted once by finite differences; one pass over those
   cached features then yields the exact gradient for all weights at once,
   and Adam steps inside a trust region, re-extracting whenever the weights
   drift too far from the extraction point. The maths and the design are in
   [gradient-tuner.md](gradient-tuner.md).

Each outer iteration prints an honest `real MSE` line (the error at the
current weights, falling over time) and rewrites `gradient_params.txt` in the
working directory, so an interrupted run loses at most the current iteration.

### Things to know

- **`gradient_params.txt` is overwritten by every run.** Copy the result of a
  finished run somewhere safe before starting another.
- **Small datasets overfit.** With ~830 free weights, tuning against a few
  thousand positions memorizes noise. Use the `max-positions` cap for
  smoke tests only, never for real tuning.
- **Error values are only comparable within one run** - each run re-fits K,
  and a different K shifts the error scale slightly.
- The tuner changes nothing about the engine. Weights live in its process
  memory only; to make the engine use them, paste the tuned values into
  `include/score.h` as the new defaults, rebuild, and validate with a
  fastchess match against the previous baseline (see the main README's
  measured-progress table for how results are recorded).
