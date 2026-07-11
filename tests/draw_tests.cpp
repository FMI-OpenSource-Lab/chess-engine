#include <gtest/gtest.h>

#include <deque>

#include "test_common.h"
#include "uci.h"

using namespace KhaosChess;

namespace {

class DrawTest : public ::testing::Test {
   protected:
    static void SetUpTestSuite() {
        init_engine_once();
    }

    // Plays a sequence of UCI moves, keeping every MoveInfo alive
    static void play(Position& pos, std::deque<MoveInfo>& infos,
                     std::initializer_list<const char*> moves) {
        for (const char* uci : moves) {
            Move m = parse_move(uci, pos);
            ASSERT_NE(m, Move::invalid_move()) << "illegal move: " << uci;
            infos.emplace_back();
            pos.do_move(m, infos.back());
        }
    }
};

TEST_F(DrawTest, StartposIsNotADraw) {
    Position pos;
    MoveInfo mi{};
    pos.set(kStartPos, &mi);

    EXPECT_FALSE(pos.is_draw());
}

TEST_F(DrawTest, KnightShuffleRepeats) {
    Position pos;
    std::deque<MoveInfo> infos(1);
    pos.set(kStartPos, &infos.back());

    // Out and back: one ply before closing the loop it is not yet a draw
    play(pos, infos, {"g1f3", "g8f6", "f3g1"});
    EXPECT_FALSE(pos.is_draw());

    play(pos, infos, {"f6g8"});  // startpos again -> repetition
    EXPECT_TRUE(pos.is_draw());
}

TEST_F(DrawTest, FiftyMoveRule) {
    Position pos;
    std::deque<MoveInfo> infos(1);
    pos.set("7k/8/8/8/8/8/8/R6K w - - 99 1", &infos.back());

    EXPECT_FALSE(pos.is_draw());  // 99 half-moves: not yet

    play(pos, infos, {"a1a2"});  // quiet move -> 100
    EXPECT_TRUE(pos.is_draw());
}

TEST_F(DrawTest, CaptureResetsFiftyMoveCounter) {
    Position pos;
    std::deque<MoveInfo> infos(1);
    pos.set("7k/8/8/8/8/8/r7/R6K w - - 99 1", &infos.back());

    play(pos, infos, {"a1a2"});  // rook takes rook
    EXPECT_FALSE(pos.is_draw());
}

}  // namespace
