#pragma once

#include "defs.h"
#include "position.h"
#include "movegen.h"
#include "score.h"

#include <vector>
#include <limits>
#include <algorithm>
#include <chrono>

namespace KhaosChess
{
    struct SearchInfo
    {
        std::vector<Move> pv;           // Principal variation
        uint64_t nodes;                 // Number of nodes searched
        uint64_t qnodes;                // Number of quiescence nodes searched
        int depth;                      // Current search depth
        std::chrono::milliseconds time; // Time spent searching
        bool stopped;                   // Whether search was stopped early

        SearchInfo() : nodes(0), qnodes(0), depth(0), stopped(false) {}
    };

    class SearchEngine
    {
    public:
        explicit SearchEngine(Position &pos);

        // Search entry point
        Value search(int depth, SearchInfo &info);

        // Set maximum time for search
        void set_max_time(std::chrono::milliseconds max_time);

    private:
        Position &pos;
        std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
        std::chrono::milliseconds max_time;
        bool should_stop;

        // Core search functions
        Value negamax(int depth, Value alpha, Value beta, SearchInfo &info, std::vector<Move> &pv);
        Value quiescence(Value alpha, Value beta, SearchInfo &info);

        // Utility functions
        bool is_time_up();
        bool is_capture(Move move);

        void score_moves(ScoredMoves *begin, ScoredMoves *end);
        void update_pv(std::vector<Move> &pv, Move move, const std::vector<Move> &child_pv);
    };

    static void search_position(Position &pos, int depth)
    {
        // one MoveInfo per move in the history stack (already in uci_loop)
        static MoveInfo dummy_mi;

        // build engine and info
        SearchEngine engine(pos);
        SearchInfo info;

        // optional: set a time limit
        // engine.set_max_time(std::chrono::milliseconds(3000));

        // run the search
        Value score = engine.search(depth, info);

        // output best move
        if (!info.pv.empty())
        {
            // assumes Move::uci() or similar returns e.g. "e2e4"
            std::cout << "bestmove " << info.pv[0] << "\n";
        }
        else
        {
            // no legal moves (checkmate or stalemate)
            std::cout << "bestmove 0000\n";
        }

        // flush so GUI sees it immediately
        std::cout.flush();
    }
} // namespace KhaosChess