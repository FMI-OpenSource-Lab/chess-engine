#include "thread.h"

#include <string>
#include <thread>
#include <vector>

#include "tt.h"

namespace KhaosChess {

namespace {
// Halts the workers and joins them on scope exit, on every path including an
// exception unwinding out of the search: a joinable std::thread left to
// destruct calls std::terminate(). Signalling stop first keeps the join from
// blocking on a helper that is still searching to its own limit.
struct WorkerGuard {
    std::vector<std::thread>& workers;
    ~WorkerGuard() {
        SearchEngine::stop();
        for (std::thread& t : workers) {
            if (t.joinable()) {
                t.join();
            }
        }
    }
};

// Lazy SMP move selection: prefer the worker that finished the deepest
// iteration, breaking ties on score. Workers that never completed depth 1
// (empty pv) are skipped. Returns the index into results (0 is the main
// worker); with a single worker this is always 0.
std::size_t pick_best_thread(const std::vector<SearchInfo>& results) {
    std::size_t best = 0;
    for (std::size_t i = 0; i < results.size(); ++i) {
        if (results[i].pv.empty()) {
            continue;
        }
        if (results[best].pv.empty() ||
            results[i].completed_depth > results[best].completed_depth ||
            (results[i].completed_depth == results[best].completed_depth &&
             results[i].score > results[best].score)) {
            best = i;
        }
    }
    return best;
}
}  // namespace

ThreadPool Threads;
std::mutex io_mutex;

SearchInfo ThreadPool::run(Position& root, const SearchLimits& limits) {
    // Note: the abort flag is cleared by the caller before this runs, not
    // here. Clearing on the search thread would race with a "stop" raised on
    // the UCI thread and could silently swallow it.
    tt::TT.new_search();  // one generation bump per search, shared by all workers

    // Helpers search the same root in parallel, sharing only the
    // transposition table; each gets its own board cloned via FEN and writes
    // its result into its own slot for the best-thread vote below.
    std::string root_fen = root.get_fen();
    std::vector<std::thread> helpers;
    std::vector<SearchInfo> results(static_cast<std::size_t>(count_));
    WorkerGuard guard{helpers};  // stops + joins on an exception path

    for (std::int32_t i = 1; i < count_; ++i) {
        helpers.emplace_back([root_fen, limits, i, &results]() {
            MoveInfo mi;
            Position p;
            p.set(root_fen, &mi);

            SearchEngine helper(p, i);
            helper.set_max_time(limits.max_time);
            helper.set_ponder(limits.ponder);
            if (limits.node_limit) {
                helper.set_max_nodes(limits.node_limit);
            }

            helper.search(limits.depth, results[i]);
        });
    }

    SearchEngine engine(root);
    engine.set_max_time(limits.max_time);
    engine.set_soft_time(limits.soft_time);
    engine.set_ponder(limits.ponder);
    if (limits.node_limit) {
        engine.set_max_nodes(limits.node_limit);
    }

    engine.search(limits.depth, results[0]);

    // The main worker is done: halt the helpers and join them before reading
    // their slots (the join synchronises those writes into this thread).
    SearchEngine::stop();
    for (std::thread& t : helpers) {
        if (t.joinable()) {
            t.join();
        }
    }

    std::size_t best = pick_best_thread(results);

    // Only the main worker reports info lines during the search. If a helper
    // won the vote, its PV was never printed, so emit a final info line for it
    // now — otherwise the GUI's last PV would not match the bestmove we play.
    if (best != 0) {
        const SearchInfo& b = results[best];
        SearchEngine::report_iteration(b, b.completed_depth, b.score);
    }

    return results[best];
}

}  // namespace KhaosChess
