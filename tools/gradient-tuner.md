# Gradient-based Texel Tuner - design & maths

This document describes the algorithm behind `tools/tuner.cpp`. It is meant
to be read alongside the code and to be a place to record ideas for improving
it. Nothing here changes how the engine plays; it is about how the evaluation
*weights* are fitted.

The previous tuner fitted the same weights by **coordinate descent**; it was
replaced by this one and lives in git history (`git show d50c8ae:tools/tuner.cpp`).
This one fits them with an **analytic gradient** (and, optionally,
Newton/IRLS). They optimise the *same* objective - they are two different
solvers for one problem, and we race them from the same starting weights.

---

## 0. What "Texel tuning" is (and isn't)

Texel tuning is a *method*, not an optimiser. The method:

1. Take a large set of quiet positions, each labelled with the game's result
   `r ∈ {0, ½, 1}` from White's point of view.
2. Push the static eval through a logistic (sigmoid) to turn "centipawns" into
   "probability White wins".
3. Fit the single scale constant `K` of that sigmoid.
4. Adjust the weights so the predicted win-probabilities best match the actual
   results, measured by mean squared error (MSE).

It says **nothing** about *how* you minimise the MSE. Coordinate descent is one
way; gradient descent / Newton is another. Both are Texel tuners.

---

## 1. Objective

For `N` positions with results `rₛ` and White-relative eval `eₛ(w)` (a function
of the 829-vector of weights `w`):

```
σ(e)  = 1 / (1 + 10^(−K·e/400))        logistic: eval → win probability
E(w)  = (1/N) Σₛ ( rₛ − σ(K·eₛ) )²      mean squared error
```

We want the `w` minimizing `E`. `K` is fit once by a 1-D grid search
(`fit_k`) to whatever weights are loaded, then **frozen** for the run - exactly
as coordinate descent does, so the race is fair. `K` is not a tuned weight; it
is the scale that converts the eval unit into a probability, a property of the
dataset and the eval scale.

---

## 2. Eval is linear in the weights

The static eval is a **sum of weighted terms** - material counts × piece values,
piece-square-table lookups, mobility × mobility weight, and so on. Every tunable
weight enters *linearly*. So, holding the discrete branch choices fixed (the
king-shelter `std::max`, which endgame evaluator fires), at a given position:

```
eₛ  =  bₛ  +  Σᵢ fₛ,ᵢ · wᵢ
```

- `fₛ,ᵢ = ∂eₛ/∂wᵢ` - the **feature**: how many times weight `i` fires in
  position `s`, already folded with the phase taper and the side-to-move sign.
- `bₛ` - the weight-*free* part: KPK bitbase scores, hardcoded mate values, plus
  the constant that closes the identity at the extraction point.

Once we know `f` and `b` for a position, its eval is a **dot product**. We never
call the real (expensive) eval again during optimisation - only during feature
extraction. This is the whole reason the gradient method is fast.

---

## 3. Feature extraction (finite differences)

We obtain each coefficient by nudging one weight and reading the real eval
(`extract_features`):

```
fₛ,ᵢ = [ eₛ(wᵢ + D) − eₛ(wᵢ − D) ] / (2D)      central difference
```

Because each term is exactly linear in `wᵢ` (within a branch), *any* `D` gives
the exact slope - no approximation error. We use **D = 24** because the tapered
eval computes `(mg·phase + eg·(24−phase)) / 24` with **integer** division; a `D`
that is a multiple of 24 makes the numerator change by a clean multiple of 24,
so the floor contributes zero rounding. The difference is exact, not "close".

Then `bₛ` closes the identity at the current point `w⁰`:

```
bₛ = eₛ(w⁰) − Σᵢ fₛ,ᵢ · w⁰ᵢ
```

By construction `bₛ + fₛ·w⁰ = eₛ(w⁰)` to floating-point noise (the
`reconstruction error ≈ 5e-12` check).

**Parallelism.** The loop is *param-major*: bump weight `i` once (global state),
then evaluate all positions in parallel with weights frozen, record the deltas,
restore. That is `1 + 2·829` parallel dataset passes ≈ **one coordinate-descent
sweep**, paid **once** per re-anchor. Each position ends up with ~55 nonzero
`(i, f)` pairs (sparse).

---

## 4. Analytic gradient

Minimize `E` using the linear model `eₛ = bₛ + fₛ·w`. Let `σₛ = σ(K·eₛ)`.

Logistic derivative (with `z = K·e/400`, so `σ = 1/(1+10^−z)`):

```
dσ/dz = ln(10) · σ(1−σ)
```

(the `ln 10` comes from the base-10 exponent, `10^−z = e^(−z·ln10)`). Chain
through `z = K·e/400` and `e = b + f·w`:

```
∂σₛ/∂wⱼ = σₛ(1−σₛ) · ln(10) · (K/400) · fₛ,ⱼ
```

Differentiate the squared residual `(rₛ − σₛ)²`:

```
∂E/∂wⱼ = (1/N) Σₛ 2(σₛ − rₛ)·σₛ(1−σₛ)·(ln10·K/400)·fₛ,ⱼ
```

Define the **per-sample scalar** (independent of `j`):

```
gₛ = 2(σₛ − rₛ)·σₛ(1−σₛ)·(ln10·K/400)
```

Then `∂E/∂wⱼ = (1/N) Σₛ gₛ·fₛ,ⱼ`. One pass over the cached features (compute
`gₛ` in parallel, then scatter `gₛ·fₛ` into the gradient) yields **all 829
components at once** (`gradient`).

**Reading `gₛ`:**
- `(σₛ − rₛ)` - prediction error (predicted win-prob minus actual result).
- `σₛ(1−σₛ)` - sigmoid sensitivity: maximal near ½, vanishing near 0/1. Uncertain
  positions teach hard; decided blowouts barely nudge the weights. Automatic
  confidence weighting.
- `ln10·K/400` - the constant from the sigmoid scale.

**Why ~100× cheaper than coordinate descent.** Coordinate descent re-evaluates
the whole dataset to decide *one* weight's move, and only ever moves along one
axis (Gauss-Seidel). The gradient method gets the true steepest-descent
direction - all axes at once - from one pass of cached dot-products, with no real
eval in the loop. Real eval is paid only at extraction.

---

## 5. Adam: Adaptive Moment Estimation

Plain gradient descent `w ← w − lr·g` needs one learning rate for all 829
weights, which span material ~5000 down to king penalties ~10. No single `lr`
serves both - big enough for the shallow directions overshoots the steep ones;
small enough for the steep ones crawls on the shallow ones (ill-conditioning).

Adam gives each weight its own adaptive step:

```
m ← β₁·m + (1−β₁)·g        1st moment  - smoothed gradient (momentum, picks DIRECTION)
v ← β₂·v + (1−β₂)·g²       2nd moment  - smoothed squared gradient (picks STEP SIZE)
m̂ = m/(1−β₁ᵗ),  v̂ = v/(1−β₂ᵗ)   bias correction for the zero start
w ← w − lr · m̂/(√v̂ + ε)
```

The `m̂/√v̂` ratio is **scale-invariant** - whether a weight's gradient is huge or
tiny, the step is ~`lr` in that weight's own units - so one `lr = 1.0` works
across the whole vector without per-weight tuning. `β₁=0.9`, `β₂=0.999`, `ε=1e-8`.

---

## 6. Re-extraction / trust region

The linear model is exact at the extraction point but the eval has mild
second-order curvature (a bilinear king-safety term, almost certainly). Measured:
the linearisation error grows as **step²** (quadratic) - at ±2 per weight the mean
error is ~1 eval unit; at ±40 it is ~294. So the features are trustworthy only
*near* the anchor.

Hence the outer loop (`optimize`, milestone 2b):

```
fit K once, freeze
repeat:
    extract features at the current weights      ← re-anchor (real eval == model)
    run Adam on the cached model until the weights drift past the trust radius
                                                   (or the model MSE plateaus)
until the anchor's real MSE stops improving
```

The `trust` radius caps how far any weight may move before we re-extract. Because
at each re-anchor the model equals the real eval exactly, the sequence of
`outer N: real MSE` lines is the **honest convergence curve** - directly
comparable to coordinate descent's numbers. (Smoke test on 10k: model prediction
and real MSE agreed to four decimals at every re-anchor.)

Tunables (CLI): `max-outer` (ceiling on re-anchors) and `trust` (drift radius).
Larger `trust` = fewer extractions but a staler model per step; smaller = more
extractions, more faithful. `trust ≈ 30` kept model≈real to ~1e-4 MSE in testing.

---

## 7. Newton / IRLS (future extension, not yet implemented)

Adam uses only the slope. With features cached we can cheaply build the
**curvature of the loss** and take Newton steps. Write `ρₛ = rₛ − σₛ` and
`aₛ = σₛ(1−σₛ)·(ln10·K/400)`, so `∇σₛ = aₛ·fₛ`. The Gauss-Newton Hessian (drop
the small residual-second-derivative term) is:

```
H  ≈  (2/N) Σₛ aₛ² · fₛ fₛᵀ        829×829, one pass of outer products
```

Each iteration solves the coupled system:

```
H·δ = −∇E ,   w ← w + δ      (+ λI damping = Levenberg-Marquardt trust region)
```

`H` is positive semidefinite by construction (sum of outer products), so a
Cholesky solve works and it can only point toward a *minimum*, never a saddle.
`n = 829` makes the `O(n³)` solve a few milliseconds. This resolves **correlated
weights** (mobility vs PSQT-centre overlap) in one joint step, where gradient
descent needs many small corrective ones - the direct-solve counterpart to
coordinate descent's Gauss-Seidel.

### The Newton-Raphson connection

Optimisation-Newton *is* Newton-Raphson applied one derivative up. Root finding
is `x ← x − g/g'`. A minimum is a stationary point, i.e. a root of the
derivative, so minimising `f` means solving `f'(x) = 0`; feed `g = f'` into
Newton-Raphson and you get `x ← x − f'/f''`. In N-D the "derivative of the
gradient" is the Hessian, giving `w ← w − H⁻¹∇E`. The one practical difference:
root-finding lands on *any* stationary point (min, max, or saddle), which is why
we use the positive-semidefinite Gauss-Newton `H` - it is inherently
minimum-seeking. That iteration (repeatedly solving an `aₛ²`-weighted
least-squares system) is exactly what **IRLS** means.

---

## 8. Code map

| Concept | Function in `tools/tuner.cpp` |
|---|---|
| Linear model `b + Σ f·w` | `model_eval` |
| Feature extraction (§3) | `extract_features` |
| Fit + freeze K (§1) | `fit_k` (via `real_mse`) |
| Honest, CD-comparable MSE | `real_mse` |
| Cheap surrogate MSE | `model_mse` |
| Analytic gradient (§4) | `gradient` |
| Round floats → integer registry | `push_weights` |
| Adam + trust region + re-extraction (§5-6) | `optimize` |

Weights are exposed through the shared registry (`include/tune.h`,
`src/tune.cpp`); output is `gradient_params.txt` in the same `name value` format
as `tuner.cpp`, so paste-back into `include/score.h` is unchanged.

---

## 9. Validation

1. **Gradient correctness** - model MSE must fall monotonically; `model MSE ≈
   real MSE` while inside the trust region. (Passed.)
2. **The MSE race** - full 725k from the 2.7.0 seed; compare final MSE and wall
   time against coordinate descent's **0.0565092**. Same objective, same start,
   different solver. Seed the 2.7.0 weights by building against
   `git show d50c8ae:include/score.h` (the pre-paste-back values with the current
   registry structure - a plain `30033ac` checkout has no registry).
3. **Games** - paste `gradient_params.txt` into `score.h`, rebuild, and beat the
   frozen base in a fastchess match. Lower MSE is a proxy; winning is the
   objective.

---

## 10. Ideas / room to improve

Notes for future work - untested unless stated:

- **Newton/IRLS** (§7) - the biggest lever; fewer, larger, curvature-aware steps.
- **Line search** along `−∇E` (or the Newton direction) using the *model* MSE, to
  choose step size instead of a fixed trust radius.
- **Subsampled extraction** - extraction dominates wall time. Re-anchor on a
  random subset of positions (features are structural counts, mostly
  position-stable), and only do a full extraction occasionally. Trades a little
  fidelity for many fewer full passes.
- **Re-fit K** every few outer iterations instead of freezing it, once the eval
  scale has shifted a lot (note: coordinate descent freezes K, so for the race
  keep it frozen; this is a post-race idea).
- **Model the bilinear term explicitly** - if the king-safety product is the only
  real nonlinearity, capturing it directly would let the trust radius grow and
  cut re-extractions.
- **Atomic save** - `save_params` (in `tuner.cpp`) truncates in place; a `.tmp` +
  `std::rename` would make interrupted runs safe. Same idea applies here.
- **Sparse Hessian** - most feature pairs never co-occur, so `H` is sparse;
  a sparse Cholesky would scale further if the weight count grows.
