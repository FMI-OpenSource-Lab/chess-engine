#include <chrono>
#include <cmath>
#include <deque>
#include <fstream>
#include <iostream>
#include <string>
#include <execution>
#include <numeric>

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
    for (char c : fen)
        fields += (c == ' ');
    if (fields == 4)
        fen += " 0 1";
    return fen;
}

double sigmoid(double eval, double k) {
    return 1.0 / (1.0 + std::pow(10.0, -k * eval / 400.0));
}

Value evaluate_white(const Position& pos) {
    Scorer<SC_ALL> scorer;
    Value v = scorer.get_score(pos);
    return pos.side_to_move() == WHITE ? v : -v;
}

double mean_squared_error(double k) {
    double total = std::transform_reduce(
        std::execution::par, samples.begin(), samples.end(), 0.0,
        std::plus<>(),
        [k](const Sample& s) {
            double diff = s.result - sigmoid(evaluate_white(s.pos), k);
            return diff * diff;
        });
    return total / double(samples.size());
}

size_t load_dataset(const char* path, size_t limit) {
    std::ifstream in(path);
    std::string line, fen;
    double result;
    while (std::getline(in, line)) {
        if (limit && samples.size() >= limit)
            break;
        if (!parse_line(line, fen, result))
            continue;
        samples.emplace_back();
        Sample& s = samples.back();
        s.result = result;
        s.pos.set(normalized(fen), &s.mi);
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

// Resume support: load a previous run's tuned_params.txt back into the
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
        std::cerr
            << "usage: tuner <positions-file> [max-positions] [resume-file]\n"
            << "       max-positions 0 = no limit\n";
        return 1;
    }

    Bitboards::init();
    BitBase::init();
    Endgames::init();
    Zobrist::init();

    size_t limit = argc > 2 ? std::stoul(argv[2]) : 0;
    std::cout << "loaded " << load_dataset(argv[1], limit) << " positions\n";

    if (argc > 3)
        std::cout << "resumed " << load_params(argv[3]) << " weights from "
                  << argv[3] << "\n";

    double k = fit_k();
    tune_weights(k);
    save_params();

    std::cout << "done - tuned weights in tuned_params.txt\n";
    return 0;
}
