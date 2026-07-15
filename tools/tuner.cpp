#include <chrono>
#include <cmath>
#include <deque>
#include <fstream>
#include <iostream>
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
struct Sample {
    Position pos;
    MoveInfo mi;
    double result;  // game result from White's view: 1.0 / 0.5 / 0.0
};

// deque, not vector: Position keeps a pointer to its MoveInfo, so
// element addresses must never move
std::deque<Sample> samples;

double sigmoid(double eval, double k) {
    return 1.0 / (1.0 + std::pow(10.0, -k * eval / 400.0));
}

Value evaluate_white(const Position& pos) {
    Scorer<SC_ALL> scorer;
    Value v = scorer.get_score(pos);
    return pos.side_to_move() == WHITE ? v : -v;
}

double mean_squared_error(double k) {
    double total = 0.0;
    for (const Sample& s : samples) {
        double diff = s.result - sigmoid(evaluate_white(s.pos), k);
        total += diff * diff;
    }
    return total / double(samples.size());
}

size_t load_dataset(const char* path, size_t limit) {
    std::ifstream in(path);
    std::string line;
    while (std::getline(in, line)) {
        if (limit && samples.size() >= limit)
            break;
        size_t sep = line.rfind(';');
        if (sep == std::string::npos)
            continue;
        samples.emplace_back();
        Sample& s = samples.back();
        s.result = std::stod(line.substr(sep + 1));
        s.pos.set(line.substr(0, sep), &s.mi);
    }
    return samples.size();
}

double fit_k() {
    double best_k = 0.05, best_e = 1e18;
    for (double k = 0.05; k <= 2.0; k += 0.05) {
        double e = mean_squared_error(k);
        if (e < best_e) {
            best_e = e;
            best_k = k;
        }
    }
    for (double k = best_k - 0.045; k <= best_k + 0.045; k += 0.005) {
        double e = mean_squared_error(k);
        if (e < best_e) {
            best_e = e;
            best_k = k;
        }
    }
    std::cout << "K = " << best_k << ", initial error = " << best_e << "\n";
    return best_k;
}

void save_params() {
    std::ofstream out("tuned_params.txt");
    print_params(out);
}

void tune_weights(double k) {
    double best_e = mean_squared_error(k);

    for (Value step : {15, 5, 2}) {
        bool improved = true;
        while (improved) {
            improved = false;
            int adjusted = 0;
            auto start = std::chrono::steady_clock::now();

            for (const TunableParam& p : tunable_params()) {
                Value original = *p.slot;
                *p.slot = original + step;
                double e = mean_squared_error(k);
                if (e >= best_e) {
                    *p.slot = original - step;
                    e = mean_squared_error(k);
                }
                if (e < best_e) {
                    best_e = e;
                    improved = true;
                    ++adjusted;
                } else {
                    *p.slot = original;
                }
            }

            auto minutes = std::chrono::duration_cast<std::chrono::minutes>(
                               std::chrono::steady_clock::now() - start)
                               .count();
            save_params();
            std::cout << "step " << step << ": " << adjusted
                      << " weights adjusted, error " << best_e << " ("
                      << minutes << " min)" << std::endl;
        }
    }
}
}  // namespace

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "usage: tuner <positions-file> [max-positions]\n";
        return 1;
    }

    Bitboards::init();
    BitBase::init();
    Endgames::init();
    Zobrist::init();

    size_t limit = argc > 2 ? std::stoul(argv[2]) : 0;
    std::cout << "loaded " << load_dataset(argv[1], limit) << " positions\n";

    double k = fit_k();
    tune_weights(k);
    save_params();

    std::cout << "done - tuned weights in tuned_params.txt\n";
    return 0;
}
