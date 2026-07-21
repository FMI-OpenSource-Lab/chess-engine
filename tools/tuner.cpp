#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <deque>
#include <execution>
#include <fstream>
#include <functional>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

#include "bitboard.h"
#include "endgame.h"
#include "position.h"
#include "score.h"
#include "tune.h"
#include "zobrist.h"

using namespace KhaosChess;

namespace {

// Finite-difference step. A multiple of 24 (the taper denominator) makes the
// difference exact: no rounding in (mg*phase + eg*(24-phase))/24.
constexpr Value D = 24;

struct Feature {
    int idx;
    double coef;
};

struct Sample {
    Position pos;
    MoveInfo mi;
    double result;               // game result from White's view: 1 / 0.5 / 0
    double eval0;                // real eval at the extraction point w0
    double base;                 // eval0 - dot(coef, w0): the weight-free part
    double g = 0.0;              // per-sample loss-gradient scalar (gradient())
    double tmp_plus, tmp_minus;  // scratch during extraction
    std::vector<Feature> feats;  // sparse nonzero coefficients
};

// deque, not vector: Position holds a pointer to its MoveInfo, so
// element addresses must never move
std::deque<Sample> samples;

// Parse one dataset line. Accepted dialects:
//   zurichess EPD:   <fen fields> c9 "1-0";      (also 0-1, 1/2-1/2)
//   Ethereal book:   <fen> [1.0]                 (also 0.5, 0.0)
//   plain:           <fen>;1.0
// The result is always from White's perspective in all three.
bool parse_line(const std::string& line, std::string& fen, double& result) {
    size_t pos;
    if ((pos = line.find(" c9 \"")) != std::string::npos) {
        std::string r =
            line.substr(pos + 5, line.find('"', pos + 5) - (pos + 5));
        if (r == "1-0")
            result = 1.0;
        else if (r == "0-1")
            result = 0.0;
        else if (r == "1/2-1/2")
            result = 0.5;
        else
            return false;
        fen = line.substr(0, pos);
        return true;
    }
    if ((pos = line.find(" [")) != std::string::npos) {
        result = std::stod(line.substr(pos + 2));
        fen = line.substr(0, pos);
        return true;
    }
    if ((pos = line.rfind(';')) != std::string::npos) {
        result = std::stod(line.substr(pos + 1));
        fen = line.substr(0, pos);
        return true;
    }
    return false;
}

// EPD lines carry only the first four FEN fields; Position::set
// expects all six, so append neutral move counters when missing
std::string normalized(std::string fen) {
    int fields = 1;
    for (char c : fen) fields += (c == ' ');
    if (fields == 4) fen += " 0 1";
    return fen;
}

Value evaluate_white(const Position& pos) {
    Scorer<SC_ALL> scorer;
    Value v = scorer.get_score(pos);
    return pos.side_to_move() == WHITE ? v : -v;
}

size_t load_dataset(const char* path, size_t limit) {
    std::ifstream in(path);
    std::string line, fen;
    double result;
    while (std::getline(in, line)) {
        if (limit && samples.size() >= limit) break;
        if (!parse_line(line, fen, result)) continue;
        samples.emplace_back();
        Sample& s = samples.back();
        s.result = result;
        s.pos.set(normalized(fen), &s.mi);
    }
    return samples.size();
}

// Resume support: load a previous run's gradient_params.txt back into the
// registry, so tuning continues from there instead of the compiled-in
// defaults
size_t load_params(const char* path) {
    std::ifstream in(path);
    std::string name;
    Value v;
    size_t applied = 0;
    while (in >> name >> v) {
        if (set_param(name, v))
            ++applied;
        else
            std::cerr << "unknown param: " << name << "\n";
    }
    return applied;
}

std::vector<double> current_weights() {
    const auto& reg = tunable_params();
    std::vector<double> w(reg.size());
    for (size_t i = 0; i < reg.size(); ++i) w[i] = double(*reg[i].slot);
    return w;
}

double model_eval(const Sample& s, const std::vector<double>& w) {
    double v = s.base;
    for (const Feature& f : s.feats) v += f.coef * w[f.idx];
    return v;
}

void extract_features() {
    const auto& reg = tunable_params();

    std::for_each(std::execution::par, samples.begin(), samples.end(),
                  [](Sample& s) {
                      s.eval0 = evaluate_white(s.pos);
                      s.feats.clear();
                  });

    for (size_t i = 0; i < reg.size(); ++i) {
        Value w0 = *reg[i].slot;

        *reg[i].slot = w0 + D;
        std::for_each(std::execution::par, samples.begin(), samples.end(),
                      [](Sample& s) { s.tmp_plus = evaluate_white(s.pos); });

        *reg[i].slot = w0 - D;
        std::for_each(std::execution::par, samples.begin(), samples.end(),
                      [](Sample& s) { s.tmp_minus = evaluate_white(s.pos); });

        *reg[i].slot = w0;  // restore before the next weight

        std::for_each(std::execution::par, samples.begin(), samples.end(),
                      [i](Sample& s) {
                          double coef = (s.tmp_plus - s.tmp_minus) / (2.0 * D);
                          if (coef != 0.0) s.feats.push_back({int(i), coef});
                      });
    }

    std::vector<double> w = current_weights();
    std::for_each(std::execution::par, samples.begin(), samples.end(),
                  [&w](Sample& s) {
                      double dot = 0.0;
                      for (const Feature& f : s.feats) dot += f.coef * w[f.idx];
                      s.base = s.eval0 - dot;
                  });
}

double sigmoid(double eval, double k) {
    // 10^x == exp(x * ln10); exp is materially faster than base-10 pow, and
    // this runs across every position on every MSE / gradient pass.
    constexpr double ln10 = 2.302585092994045901;
    return 1.0 / (1.0 + std::exp(-k * eval * ln10 / 400.0));
}

// Real-eval MSE over the registry's current weights: the honest number
// reported at every outer anchor. Kept as a standalone reference check even
// though fit_k now caches evals; unused in the default path.
[[maybe_unused]] double real_mse(double k) {
    double total = std::transform_reduce(
        std::execution::par, samples.begin(), samples.end(), 0.0, std::plus<>(),
        [k](const Sample& s) {
            double diff = s.result - sigmoid(evaluate_white(s.pos), k);
            return diff * diff;
        });
    return total / double(samples.size());
}

double fit_k() {
    // The eval is independent of K, so evaluate every position once (cached in
    // eval0, which extract_features overwrites later) and let the grid search
    // re-run only the sigmoid over the cache -- ~38 full real-eval passes
    // collapse to one.
    std::for_each(std::execution::par, samples.begin(), samples.end(),
                  [](Sample& s) { s.eval0 = evaluate_white(s.pos); });
    auto cached_mse = [](double k) {
        double total = std::transform_reduce(
            std::execution::par, samples.begin(), samples.end(), 0.0,
            std::plus<>(), [k](const Sample& s) {
                double diff = s.result - sigmoid(s.eval0, k);
                return diff * diff;
            });
        return total / double(samples.size());
    };

    double best_k = 0.05, best_e = 1e18;
    for (double k = 0.05; k <= 2.0; k += 0.05) {
        double e = cached_mse(k);
        if (e < best_e) {
            best_e = e;
            best_k = k;
        }
    }
    for (double k = best_k - 0.045; k <= best_k + 0.045; k += 0.005) {
        double e = cached_mse(k);
        if (e < best_e) {
            best_e = e;
            best_k = k;
        }
    }
    std::cout << "K = " << best_k << ", initial error = " << best_e << "\n";
    return best_k;
}

// MSE using the cached linear model — the cheap inner-loop workhorse.
double model_mse(const std::vector<double>& w, double k) {
    double total = std::transform_reduce(
        std::execution::par, samples.begin(), samples.end(), 0.0, std::plus<>(),
        [&w, k](const Sample& s) {
            double diff = s.result - sigmoid(model_eval(s, w), k);
            return diff * diff;
        });
    return total / double(samples.size());
}

// Gradient of the model MSE w.r.t. every weight:
//   dMSE/dw_j = (1/N) Sum 2(sig-r)*sig(1-sig)*(ln10*k/400)*f_j
std::vector<double> gradient(const std::vector<double>& w, double k) {
    const double c = std::log(10.0) * k / 400.0;
    std::for_each(std::execution::par, samples.begin(), samples.end(),
                  [&w, k, c](Sample& s) {
                      double sig = sigmoid(model_eval(s, w), k);
                      s.g = 2.0 * (sig - s.result) * sig * (1.0 - sig) * c;
                  });
    // Parallel scatter: ~40M (idx, coef) updates per call over 725k positions.
    // Split the samples into chunks, accumulate a thread-local gradient per
    // chunk, then reduce -- the reduce copies only `chunks` vectors.
    const size_t n = samples.size();
    const size_t chunks = std::min<size_t>(256, std::max<size_t>(1, n));
    std::vector<size_t> ids(chunks);
    std::iota(ids.begin(), ids.end(), 0);
    std::vector<double> grad = std::transform_reduce(
        std::execution::par, ids.begin(), ids.end(),
        std::vector<double>(w.size(), 0.0),
        [](std::vector<double> a, const std::vector<double>& b) {
            for (size_t j = 0; j < a.size(); ++j) a[j] += b[j];
            return a;
        },
        [&](size_t c) {
            std::vector<double> local(w.size(), 0.0);
            for (size_t i = c * n / chunks; i < (c + 1) * n / chunks; ++i) {
                const Sample& s = samples[i];
                for (const Feature& f : s.feats) local[f.idx] += s.g * f.coef;
            }
            return local;
        });
    for (double& gj : grad) gj /= double(n);
    return grad;
}

void push_weights(const std::vector<double>& w) {
    const auto& reg = tunable_params();
    for (size_t i = 0; i < reg.size(); ++i)
        *reg[i].slot = Value(std::lround(w[i]));
}

// Write the registry to `path` atomically: full write to a temp file, then
// rename. An interrupt mid-write leaves the previous good file untouched.
void save_params_atomic(const char* path) {
    const std::string tmp = std::string(path) + ".tmp";
    {
        std::ofstream out(tmp);
        print_params(out);
    }
    std::rename(tmp.c_str(), path);
}

// Re-extraction outer loop.
//
// Adam minimises the cached linear model, but the linearisation only holds
// near the extraction point (error grows ~ step^2). So we cap how far the
// weights may drift from the current anchor (the trust region); when they hit
// that cap the inner loop stops, we re-extract features at the new point and
// continue. At every re-anchor the model equals the real eval exactly, so each
// "outer N: real MSE" line is the honest number.
void optimize(double k, int max_outer, double trust) {
    const auto t_start = std::chrono::steady_clock::now();
    auto minutes = [&t_start]() {
        return std::chrono::duration_cast<std::chrono::minutes>(
                   std::chrono::steady_clock::now() - t_start)
            .count();
    };

    std::vector<double> w = current_weights();
    std::vector<double> best_w = w;
    double best_real = 1e18;

    // Adam's step at t=1 is always +-lr on EVERY weight (bias correction makes
    // mhat/sqrt(vhat) = sign(g)), so near the optimum a fixed lr overshoots.
    // Halving lr whenever an outer step fails to pay off gives a refinement
    // phase, analogous to a shrinking step schedule.
    double lr = 1.0;
    const double lr_min = 0.05;
    const int patience = 20;

    for (int outer = 1; outer <= max_outer; ++outer) {
        push_weights(w);
        w = current_weights();  // exact rounded anchor
        extract_features();     // re-anchor: real eval == model here
        std::vector<double> anchor = w;

        double anchor_real = model_mse(w, k);  // == real_mse at the anchor
        std::cout << "outer " << outer << ": real MSE " << anchor_real
                  << "  (lr " << lr << ", " << minutes() << " min)"
                  << std::endl;

        if (anchor_real < best_real) {
            best_real = anchor_real;
            best_w = w;
        } else {
            lr *= 0.5;  // step did not pay off: refine instead of giving up
            std::cout << "  no improvement -> lr " << lr << std::endl;
            if (lr < lr_min) {
                std::cout << "converged (lr floor)" << std::endl;
                break;
            }
        }

        // Inner Adam on the cached model. Keep the best point seen and allow a
        // few non-improving steps before quitting -- a single bad step means
        // nothing, and bailing on one is what stalled the first race run.
        std::vector<double> m(w.size(), 0.0), v(w.size(), 0.0);
        const double b1 = 0.9, b2 = 0.999, eps = 1e-8;
        std::vector<double> best_inner_w = w;
        double best_inner = anchor_real, drift = 0.0;
        int stale = 0, t = 1;
        for (; t <= 10000; ++t) {
            std::vector<double> grad = gradient(w, k);
            for (size_t j = 0; j < w.size(); ++j) {
                m[j] = b1 * m[j] + (1 - b1) * grad[j];
                v[j] = b2 * v[j] + (1 - b2) * grad[j] * grad[j];
                double mhat = m[j] / (1 - std::pow(b1, t));
                double vhat = v[j] / (1 - std::pow(b2, t));
                w[j] -= lr * mhat / (std::sqrt(vhat) + eps);
            }
            drift = 0.0;
            for (size_t j = 0; j < w.size(); ++j)
                drift = std::max(drift, std::fabs(w[j] - anchor[j]));
            double e = model_mse(w, k);
            if (e < best_inner) {
                best_inner = e;
                best_inner_w = w;
                stale = 0;
            } else if (++stale >= patience) {
                break;
            }
            if (drift >= trust) break;
        }
        w = best_inner_w;  // carry on from the best point, not the last one

        std::cout << "  inner: model MSE " << best_inner << " after " << t
                  << " iters, max drift " << drift << std::endl;

        // checkpoint after every outer step so an interrupted overnight run
        // still leaves the best weights on disk
        push_weights(best_w);
        save_params_atomic("gradient_params.txt");
    }

    push_weights(best_w);
    save_params_atomic("gradient_params.txt");
    std::cout << "wrote gradient_params.txt, best real MSE " << best_real
              << " (" << minutes() << " min total)" << std::endl;
}

}  // namespace

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "usage: tuner <positions-file> [max-positions] "
                     "[max-outer] [trust] [resume-file]\n"
                     "       max-positions 0 = no limit\n";
        return 1;
    }

    Bitboards::init();
    BitBase::init();
    Endgames::init();
    Zobrist::init();

    size_t limit = argc > 2 ? std::stoul(argv[2]) : 0;
    std::cout << "loaded " << load_dataset(argv[1], limit) << " positions\n";

    if (argc > 5)
        std::cout << "resumed " << load_params(argv[5]) << " weights from "
                  << argv[5] << "\n";

    double k = fit_k();
    int max_outer = argc > 3 ? std::stoi(argv[3]) : 15;
    double trust = argc > 4 ? std::stod(argv[4]) : 30.0;
    optimize(k, max_outer, trust);

    return 0;
}
