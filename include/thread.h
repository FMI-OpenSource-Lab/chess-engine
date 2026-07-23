#pragma once

#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

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

// Persistent Lazy SMP pool. The worker threads are created once (on the first
// search, or when the Threads count changes) and parked in an idle loop between
// searches, rather than spawned per move. Each worker owns its own board and
// SearchEngine and shares only the transposition table; run() wakes them all on
// the same root, waits for them, and returns the best worker's result. Worker 0
// is the "main": it honours the soft time limit and prints the live info lines.
class ThreadPool {
   public:
    ~ThreadPool();

    void set_count(std::int32_t n) {
        count_ = n < 1 ? 1 : n;
    }
    std::int32_t count() const {
        return count_;
    }

    SearchInfo run(Position& root, const SearchLimits& limits);

   private:
    struct Worker {
        std::thread thread;
        Position pos;
        MoveInfo mi;
        std::unique_ptr<SearchEngine> engine;
        SearchInfo result;
        std::int32_t id = 0;
        bool searching = false;  // guarded by mtx_
    };

    void idle_loop(Worker* w);
    void ensure_workers();  // (re)spawn so workers_.size() == count_
    void kill_workers();

    std::vector<std::unique_ptr<Worker>> workers_;  // index 0 is the main worker
    std::int32_t count_ = 1;

    std::mutex mtx_;
    std::condition_variable cv_;       // wakes parked workers to start a search
    std::condition_variable done_cv_;  // a worker signals it has finished
    SearchLimits limits_{};            // params of the current search (read by workers)
    bool exit_ = false;                // set on shutdown so idle loops return
};

extern ThreadPool Threads;  // global instance, like tt::TT

}  // namespace KhaosChess
