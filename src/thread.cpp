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
}  // namespace

ThreadPool Threads;

SearchInfo ThreadPool::run(Position& root, const SearchLimits& limits) {
    // Note: the abort flag is cleared by the caller before this runs, not
    // here. Clearing on the search thread would race with a "stop" raised on
    // the UCI thread and could silently swallow it.
    tt::TT.new_search();  // one generation bump per search, shared by all workers

    // Helpers search the same root in parallel, sharing only the
    // transposition table; each gets its own board cloned via FEN.
    std::string root_fen = root.get_fen();
    std::vector<std::thread> helpers;
    WorkerGuard guard{helpers};  // stops + joins on return or exception

    for (std::int32_t i = 1; i < count_; ++i) {
        helpers.emplace_back([root_fen, limits, i]() {
            MoveInfo mi;
            Position p;
            p.set(root_fen, &mi);

            SearchEngine helper(p, i);
            helper.set_max_time(limits.max_time);
            if (limits.node_limit) {
                helper.set_max_nodes(limits.node_limit);
            }

            SearchInfo hinfo;
            helper.search(limits.depth, hinfo);
        });
    }

    SearchEngine engine(root);
    engine.set_max_time(limits.max_time);
    engine.set_soft_time(limits.soft_time);
    if (limits.node_limit) {
        engine.set_max_nodes(limits.node_limit);
    }

    SearchInfo info;
    engine.search(limits.depth, info);

    return info;  // guard stops the helpers and joins them here
}

}  // namespace KhaosChess
