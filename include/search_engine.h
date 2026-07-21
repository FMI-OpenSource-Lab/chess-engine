#pragma once

#include <algorithm>
#include <chrono>
#include <limits>
#include <vector>
#include <atomic>

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

    SearchInfo() : nodes(0), q_nodes(0), depth(0), stopped(false) {}
};

class SearchEngine {
   public:
    explicit SearchEngine(Position& pos, std::int32_t id = 0);

    // Search entry point
    Value search(std::int32_t depth, SearchInfo& info);
    // Set maximum time for search
    void set_max_time(std::chrono::milliseconds max_time);

    // Cap the search at a node budget; 0 means no limit. Node-limited
    // games are immune to timing noise from CPU contention
    void set_max_nodes(std::uint64_t nodes);

    // Shared stop flag controls for Lazy SMP
    static void clear_stop();  // lower the flag before a search
    static void stop();        // raise it to halt every thread

   private:
    Position& pos;
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
    std::chrono::milliseconds max_time;
    std::uint64_t max_nodes;  // 0 = no node limit

    bool should_stop;
    std::uint64_t time_checks;

    // Core search functions
    Value negamax(std::int32_t depth, std::int32_t ply, Value alpha, Value beta,
                  SearchInfo& info, bool can_null = true,
                  std::int32_t num_extensions = 0,
                  Move prev_move = Move::invalid_move());
    Value quiescence(std::int32_t ply, Value alpha, Value beta,
                     SearchInfo& info);

    static std::atomic<bool> abort_search;
    int thread_id;  // 0 = main worker (reports, searches every depth)

    // Utility functions
    bool is_time_up();
    bool is_capture(Move move);
    bool null_move_cuts(std::int32_t depth, std::int32_t ply, Value beta,
                        SearchInfo& info, std::int32_t num_extensions);
    bool should_skip_quiet(std::int32_t depth, std::int32_t moves_searched,
                           Value alpha, Value static_eval);

    void score_moves(ScoredMoves* begin, ScoredMoves* end, std::int32_t ply,
                     Move tt_move = Move::invalid_move(),
                     bool score_quiets = true,
                     Move prev_move = Move::invalid_move());
    void update_quiet_stats(Move move, std::int32_t ply, std::int32_t depth,
                            Move prev_move, Move* searched_quiets,
                            std::int32_t num_quiets);
    void update_pv(Move move, std::int32_t ply);
    void report_iteration(const SearchInfo& info, std::int32_t depth,
                          Value score) const;

    Value aspiration_search(std::int32_t depth, Value prev_score,
                            SearchInfo& info);
    Value pvs_search(std::int32_t depth, std::int32_t ply, Value alpha,
                     Value beta, SearchInfo& info, std::int32_t moves_searched,
                     bool in_check, bool is_quiet, std::int32_t num_extensions,
                     Move prev_move);

    std::int32_t lmr_reduction(std::int32_t depth, std::int32_t moves_searched,
                               bool in_check, bool is_quiet);
    std::int32_t calculate_extension_depth(bool in_check,
                                           std::int32_t num_extensions) const;

    // Quiet-move ordering: two killer slots per ply, and a butterfly
    // history table bumped by depth^2 on every quiet beta cutoff
    Move killers[MAX_PLY][2];
    std::int32_t history[BOTH][SQUARE_TOTAL][SQUARE_TOTAL];

    // Continuation history: how well a quiet move did as a reply to the
    // previous move, keyed by (prev piece, prev target) then (this piece,
    // this target). The colored Piece index folds in the side to move.
    std::int16_t continuation_history[PIECE_NB][SQUARE_TOTAL][PIECE_NB]
                                     [SQUARE_TOTAL];

    // Countermove: per side, the quiet move that last produced a beta cutoff
    // in reply to the (from -> to) move the opponent just played
    Move countermove[BOTH][SQUARE_TOTAL][SQUARE_TOTAL];

    // Triangular PV table: pv_table[ply] is the best line found from that
    // ply, ending at pv_length[ply]. Copied into SearchInfo at the root.
    Move pv_table[MAX_PLY][MAX_PLY];
    std::int32_t pv_length[MAX_PLY];
};

}  // namespace KhaosChess
