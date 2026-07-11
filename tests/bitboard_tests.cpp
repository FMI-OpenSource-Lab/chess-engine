#include <gtest/gtest.h>

#include "test_common.h"

using namespace KhaosChess;

namespace {

// Sanity tests for the bitboard layer: square mapping, precomputed leaper
// tables, magic slider attacks and the bit-manipulation helpers. Expected
// squares are spelled out by hand from the A8=0 ... H1=63 board layout.
class BitboardTest : public ::testing::Test {
   protected:
    static void SetUpTestSuite() { init_engine_once(); }
};

TEST_F(BitboardTest, SquareMapping) {
    EXPECT_EQ(square_to_BB(A8), 1ULL);
    EXPECT_EQ(square_to_BB(H1), 1ULL << 63);
    EXPECT_EQ(make_square(FILE_E, RANK_4), E4);
    EXPECT_EQ(file_of(E4), FILE_E);
    EXPECT_EQ(rank_of(E4), RANK_4);
}

TEST_F(BitboardTest, KnightAttacks) {
    // Corner knight reaches only two squares
    EXPECT_EQ(attacks_bb_by<KNIGHT>(A1), square_to_BB(B3) | C2);

    // Centralized knight reaches all eight
    EXPECT_EQ(count_bits(attacks_bb_by<KNIGHT>(E4)), 8);
    EXPECT_TRUE(attacks_bb_by<KNIGHT>(E4) & D6);
    EXPECT_FALSE(attacks_bb_by<KNIGHT>(E4) & E5);
}

TEST_F(BitboardTest, KingAttacks) {
    EXPECT_EQ(attacks_bb_by<KING>(E1),
              square_to_BB(D1) | F1 | D2 | E2 | F2);
    EXPECT_EQ(count_bits(attacks_bb_by<KING>(E4)), 8);
}

TEST_F(BitboardTest, RookAttacksEmptyBoard) {
    // A rook always sees 14 squares on an empty board
    EXPECT_EQ(count_bits(attacks_bb_by<ROOK>(D4, 0)), 14);
    EXPECT_EQ(count_bits(attacks_bb_by<ROOK>(A1, 0)), 14);
}

TEST_F(BitboardTest, RookAttacksBlocked) {
    // A blocker on A3 stops the rook: file A2-A3, full first rank
    const BITBOARD occ = square_to_BB(A3);
    const BITBOARD attacks = attacks_bb_by<ROOK>(A1, occ);

    EXPECT_EQ(count_bits(attacks), 9);
    EXPECT_TRUE(attacks & A3);   // the blocker square itself is attacked
    EXPECT_FALSE(attacks & A4);  // but nothing behind it
}

TEST_F(BitboardTest, BishopAttacks) {
    EXPECT_EQ(count_bits(attacks_bb_by<BISHOP>(E4, 0)), 13);

    // Blocker on G6 cuts off H7
    const BITBOARD attacks = attacks_bb_by<BISHOP>(E4, square_to_BB(G6));
    EXPECT_TRUE(attacks & G6);
    EXPECT_FALSE(attacks & H7);
}

TEST_F(BitboardTest, QueenAttacks) {
    // Queen = rook + bishop
    EXPECT_EQ(attacks_bb_by<QUEEN>(E4, 0),
              attacks_bb_by<ROOK>(E4, 0) | attacks_bb_by<BISHOP>(E4, 0));
    EXPECT_EQ(count_bits(attacks_bb_by<QUEEN>(E4, 0)), 27);
}

TEST_F(BitboardTest, PawnAttacks) {
    EXPECT_EQ(pawn_attacks_bb(WHITE, E4), square_to_BB(D5) | F5);
    EXPECT_EQ(pawn_attacks_bb(BLACK, E4), square_to_BB(D3) | F3);

    // Edge pawns attack a single square
    EXPECT_EQ(pawn_attacks_bb(WHITE, A2), square_to_BB(B3));
    EXPECT_EQ(pawn_attacks_bb(BLACK, H7), square_to_BB(G6));
}

TEST_F(BitboardTest, BitManipulation) {
    EXPECT_EQ(count_bits(0), 0);
    EXPECT_EQ(count_bits(DarkSquares), 32);
    EXPECT_EQ(count_bits(Rank4_Bits), 8);

    EXPECT_EQ(get_ls1b(square_to_BB(E4)), E4);
    EXPECT_EQ(get_ls1b(Rank8_Bits), A8);
    EXPECT_EQ(get_msb(Rank8_Bits), H8);

    BITBOARD b = square_to_BB(D4) | G7;
    EXPECT_EQ(pop_ls1b(b), G7);  // G7 (square 14) comes before D4 (square 35)
    EXPECT_EQ(b, square_to_BB(D4));
}

TEST_F(BitboardTest, LinesAndAlignment) {
    EXPECT_EQ(distance<Square>(A1, H8), 7);
    EXPECT_EQ(distance<File>(A1, H8), 7);

    EXPECT_TRUE(are_squares_aligned(A1, H8, D4));
    EXPECT_FALSE(are_squares_aligned(A1, H8, E4));

    // in_between_bb excludes the source but includes the target
    EXPECT_TRUE(in_between_bb(A1, A8) & A4);
    EXPECT_FALSE(in_between_bb(A1, A8) & A1);
}

}  // namespace
