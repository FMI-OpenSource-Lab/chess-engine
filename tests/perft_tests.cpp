#include <gtest/gtest.h>

#include <array>
#include <chrono>

#include "perft.h"
#include "test_common.h"

using namespace KhaosChess;

namespace {

// Deep perft runs that double as a movegen speed benchmark. No time
// assertions on purpose — wall-clock limits make tests flaky on a busy
// machine; the speed is printed and recorded for manual comparison.
class PerftBenchmark : public ::testing::Test {
   protected:
    static void SetUpTestSuite() { init_engine_once(); }

    // Runs perft, verifies the count, and reports speed
    static void benchmark(const std::string& fen, std::int32_t depth,
                          std::uint64_t expected) {
        Position pos;
        MoveInfo mi{};
        pos.set(fen, &mi);

        const auto start = std::chrono::steady_clock::now();
        const std::uint64_t nodes = perft_driver(pos, depth);
        const auto elapsed =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - start);

        ASSERT_EQ(nodes, expected) << "FEN: " << fen;

        const double seconds = elapsed.count() / 1000.0;
        const double mnps = seconds > 0 ? nodes / seconds / 1e6 : 0.0;
        std::cout << "[ PERFT    ] depth " << depth << ": " << nodes
                  << " nodes in " << elapsed.count() << " ms  (" << mnps
                  << " Mnps)\n";
        RecordProperty("mnps", static_cast<int>(mnps * 1000));
    }

    // Runs the full depth ladder 1..6 on one position, one report line per
    // depth, stopping at the first depth whose node count is wrong
    static void benchmark_ladder(const std::string& fen,
                                 const std::array<std::uint64_t, 6>& expected) {
        for (std::int32_t depth = 1; depth <= 6; ++depth)
            ASSERT_NO_FATAL_FAILURE(benchmark(fen, depth, expected[depth - 1]));
    }
};

// Reference node counts from the Chess Programming Wiki. Kiwipete has no
// ladder on purpose: its depth 6 is ~8 billion nodes
TEST_F(PerftBenchmark, StartposLadder) {
    benchmark_ladder(kStartPos, {20, 400, 8902, 197281, 4865609, 119060324});
}

TEST_F(PerftBenchmark, EnPassantPinsLadder) {
    benchmark_ladder(kEnPassantPins, {14, 191, 2812, 43238, 674624, 11030083});
}

TEST_F(PerftBenchmark, PromotionsLadder) {
    benchmark_ladder(kPromotions, {6, 264, 9467, 422333, 15833292, 706045033});
}

TEST_F(PerftBenchmark, KiwipeteDepth5) {
    benchmark(kKiwipete, 5, 193690690u);
}

}  // namespace
