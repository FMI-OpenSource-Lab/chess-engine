#include <gtest/gtest.h>

#include "perft.h"
#include "test_common.h"

using namespace KhaosChess;

namespace {

// Perft node-count regression tests: each case runs a full legal move
// generation to a fixed depth on a known position and compares against
// reference values from the Chess Programming Wiki
struct PerftCase {
    std::string name;  // becomes part of the test name: letters/digits/_ only
    std::string fen;
    std::int32_t depth;
    std::uint64_t expected;
};

// Teaches gtest how to display a PerftCase (test listings, failure output);
// without this it hex-dumps the raw bytes of the struct
void PrintTo(const PerftCase& c, std::ostream* os) { *os << c.name; }

class PositionTest : public ::testing::TestWithParam<PerftCase> {
   protected:
    static void SetUpTestSuite() { init_engine_once(); }
};

TEST_P(PositionTest, NodeCount) {
    const PerftCase& c = GetParam();

    Position pos;
    MoveInfo mi{};
    pos.set(c.fen, &mi);

    EXPECT_EQ(perft_driver(pos, c.depth), c.expected) << "FEN: " << c.fen;
}

INSTANTIATE_TEST_SUITE_P(
    Positions, PositionTest,
    ::testing::Values(
        PerftCase{"StartPos_d1", kStartPos, 1, 20},
        PerftCase{"StartPos_d2", kStartPos, 2, 400},
        PerftCase{"StartPos_d3", kStartPos, 3, 8902},
        PerftCase{"StartPos_d4", kStartPos, 4, 197281},
        PerftCase{"StartPos_d5", kStartPos, 5, 4865609},

        PerftCase{"Kiwipete_d1", kKiwipete, 1, 48},
        PerftCase{"Kiwipete_d2", kKiwipete, 2, 2039},
        PerftCase{"Kiwipete_d3", kKiwipete, 3, 97862},
        PerftCase{"Kiwipete_d4", kKiwipete, 4, 4085603},

        PerftCase{"EnPassantPins_d3", kEnPassantPins, 3, 2812},
        PerftCase{"EnPassantPins_d4", kEnPassantPins, 4, 43238},
        PerftCase{"EnPassantPins_d5", kEnPassantPins, 5, 674624},

        PerftCase{"Promotions_d3", kPromotions, 3, 9467},
        PerftCase{"Promotions_d4", kPromotions, 4, 422333},

        PerftCase{"Talkchess_d3", kTalkchess, 3, 62379},
        PerftCase{"Talkchess_d4", kTalkchess, 4, 2103487}),
    [](const ::testing::TestParamInfo<PerftCase>& info) {
        return info.param.name;
    });

}  // namespace
