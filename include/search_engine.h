#pragma once

#include <algorithm>
#include <chrono>
#include <limits>
#include <vector>

#include "defs.h"
#include "movegen.h"
#include "position.h"
#include "score.h"

namespace KhaosChess {
struct SearchInfo {
    std::vector<Move> pv;            // Principal variation
    std::uint64_t nodes;             // Number of nodes searched
    std::uint64_t q_nodes;           // Number of quiescence nodes searched
    std::int32_t depth;              // Current search depth
    std::chrono::milliseconds time;  // Time spent searching
    bool stopped;                    // Whether search was stopped early

    SearchInfo() : nodes(0), q_nodes(0), depth(0), stopped(false) {
    }
};

class SearchEngine {
   public:
    explicit SearchEngine(Position& pos);

    // Search entry point
    Value search(std::int32_t depth, SearchInfo& info);

    // Set maximum time for search
    void set_max_time(std::chrono::milliseconds max_time);

   private:
    Position& pos;
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
    std::chrono::milliseconds max_time;
    bool should_stop;
    std::uint64_t time_checks;

    // Core search functions
    Value negamax(std::int32_t depth, std::int32_t ply, Value alpha, Value beta,
                  SearchInfo& info);
    Value quiescence(std::int32_t ply, Value alpha, Value beta, SearchInfo& info);

    // Utility functions
    bool is_time_up();
    bool is_capture(Move move);

    void score_moves(ScoredMoves* begin, ScoredMoves* end, Move tt_move = Move::invalid_move(), bool score_quiets = true);

    // Triangular PV table: pv_table[ply] is the best line found from that
    // ply, ending at pv_length[ply]. Copied into SearchInfo at the root.
    Move pv_table[MAX_PLY][MAX_PLY];
    std::int32_t pv_length[MAX_PLY];
};

}  // namespace KhaosChess
