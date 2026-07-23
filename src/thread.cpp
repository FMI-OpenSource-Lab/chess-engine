#include "thread.h"

#include <cstddef>

#include "tt.h"

namespace KhaosChess {

namespace {
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

ThreadPool::~ThreadPool() {
    kill_workers();
}

// A parked worker waits here for run() to arm it, runs one search, then parks
// again. It only touches its own board/engine/result, so no locking is needed
// during the search itself; the mutex just guards the searching/exit flags.
void ThreadPool::idle_loop(Worker* w) {
    while (true) {
        std::unique_lock<std::mutex> lk(mtx_);
        cv_.wait(lk, [&] { return w->searching || exit_; });
        if (exit_) {
            return;
        }
        std::int32_t depth = limits_.depth;
        lk.unlock();

        w->engine->search(depth, w->result);

        lk.lock();
        w->searching = false;
        done_cv_.notify_all();
    }
}

void ThreadPool::ensure_workers() {
    if (static_cast<std::int32_t>(workers_.size()) == count_) {
        return;
    }
    kill_workers();
    for (std::int32_t i = 0; i < count_; ++i) {
        auto w = std::make_unique<Worker>();
        w->id = i;
        w->engine = std::make_unique<SearchEngine>(w->pos, i);
        Worker* wp = w.get();
        workers_.push_back(std::move(w));
        wp->thread = std::thread(&ThreadPool::idle_loop, this, wp);
    }
}

void ThreadPool::kill_workers() {
    if (workers_.empty()) {
        return;
    }
    {
        std::unique_lock<std::mutex> lk(mtx_);
        exit_ = true;
    }
    cv_.notify_all();
    for (auto& w : workers_) {
        if (w->thread.joinable()) {
            w->thread.join();
        }
    }
    workers_.clear();
    exit_ = false;
}

SearchInfo ThreadPool::run(Position& root, const SearchLimits& limits) {
    ensure_workers();
    tt::TT.new_search();  // one generation bump per search, shared by all workers

    std::string root_fen = root.get_fen();

    // Arm every worker on the same root, then wake them together.
    {
        std::unique_lock<std::mutex> lk(mtx_);
        limits_ = limits;
        for (auto& w : workers_) {
            w->pos.set(root_fen, &w->mi);
            w->engine->set_max_time(limits.max_time);
            // Only the main worker honours the soft limit; helpers run to the
            // hard limit (or until they are stopped below).
            w->engine->set_soft_time(
                w->id == 0 ? limits.soft_time : std::chrono::milliseconds(0));
            w->engine->set_ponder(limits.ponder);
            w->engine->set_max_nodes(limits.node_limit);
            w->result = SearchInfo();
            w->searching = true;
        }
    }
    cv_.notify_all();

    // Wait for the main worker to hit its limit, then stop the helpers: they
    // ignore the soft budget and would otherwise run on to the hard limit.
    {
        std::unique_lock<std::mutex> lk(mtx_);
        done_cv_.wait(lk, [&] { return !workers_[0]->searching; });
    }
    SearchEngine::stop();
    {
        std::unique_lock<std::mutex> lk(mtx_);
        done_cv_.wait(lk, [&] {
            for (auto& w : workers_) {
                if (w->searching) {
                    return false;
                }
            }
            return true;
        });
    }

    std::vector<SearchInfo> results;
    results.reserve(workers_.size());
    for (auto& w : workers_) {
        results.push_back(w->result);
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
