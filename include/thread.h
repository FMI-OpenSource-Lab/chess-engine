#pragma once

#include <chrono>
#include <cstdint>
#include <mutex>

#include "position.h"
#include "search_engine.h"

namespace KhaosChess {

// Serialises writes to stdout: the search runs on its own thread and reports
// "info"/"bestmove" lines while the UCI loop may print "readyok" and other
// replies, so both sides lock this before touching std::cout.
extern std::mutex io_mutex;

// Everything a search run is bounded by; a zero field means "no limit"
struct SearchLimits {
    std::chrono::milliseconds max_time{0};   // hard: abort an iteration in flight
    std::chrono::milliseconds soft_time{0};  // soft: don't start a new iteration
    std::uint64_t node_limit = 0;
    std::int32_t depth = 64;
    bool ponder = false;                     // search on the opponent's clock
};

// Owns the Lazy SMP dispatch: clones the root per worker, launches the
// helper threads, runs the main search, and joins them. Returns the main
// worker's result (its pv[0] is the move to play).
class ThreadPool {
   public:
    void set_count(std::int32_t n) {
        count_ = n < 1 ? 1 : n;
    }
    std::int32_t count() const {
        return count_;
    }

    SearchInfo run(Position& root, const SearchLimits& limits);

   private:
    std::int32_t count_ = 1;
};

extern ThreadPool Threads;  // global instance, like tt::TT

}  // namespace KhaosChess
