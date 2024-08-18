// system headers
#pragma once
#ifndef DEFS_H
#define DEFS_H

#define NAME "iuli 1.0"

#include <cassert>
//#include <cstdint>

#if defined(_MSC_VER)
	// Disable some silly and noisy warnings from MSVC compiler
#pragma warning(disable: 4127)  // Conditional expression is constant
#pragma warning(disable: 4146)  // Unary minus operator applied to unsigned type
#pragma warning(disable: 4800)  // Forcing value to bool 'true' or 'false'
#endif

// define bitboard data type
#define U64 unsigned long long 

enum Square : int {
	A8, B8, C8, D8, E8, F8, G8, H8,
	A7, B7, C7, D7, E7, F7, G7, H7,
	A6, B6, C6, D6, E6, F6, G6, H6,
	A5, B5, C5, D5, E5, F5, G5, H5,
	A4, B4, C4, D4, E4, F4, G4, H4,
	A3, B3, C3, D3, E3, F3, G3, H3,
	A2, B2, C2, D2, E2, F2, G2, H2,
	A1, B1, C1, D1, E1, F1, G1, H1,
	NONE,

	SQUARE_ZERO = 0,
	SQUARE_NB = 64
};

constexpr const char* squareToCoordinates[] = {
	"a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
	"a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
	"a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
	"a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
	"a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
	"a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
	"a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
	"a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1"
};

enum PieceType : int {
	NO_PIECE_TYPE,
	PAWN,
	KNIGHT,
	BISHOP,
	ROOK,
	QUEEN,
	KING,
	ALL_PIECES = 0,
	PIECE_TYPE_NB = 8
};

enum Piece : int {
	WHITE_PAWN,
	WHITE_KNIGHT,
	WHITE_BISHOP,
	WHITE_ROOK,
	WHITE_QUEEN,
	WHITE_KING,
	BLACK_PAWN,
	BLACK_KNIGHT,
	BLACK_BISHOP,
	BLACK_ROOK,
	BLACK_QUEEN,
	BLACK_KING,
	EMPTY,
};

enum Color
{
	WHITE,
	BLACK,
	BOTH
};

enum Direction : int {
	NORTH = 8,
	EAST = 1,
	SOUTH = -NORTH,
	WEST = -EAST,

	NORTH_EAST = NORTH + EAST,
	SOUTH_EAST = SOUTH + EAST,
	SOUTH_WEST = SOUTH + WEST,
	NORTH_WEST = NORTH + WEST
};

enum File : int {
	FILE_A,
	FILE_B,
	FILE_C,
	FILE_D,
	FILE_E,
	FILE_F,
	FILE_G,
	FILE_H,
	FILE_NB
};

enum Rank : int {
	RANK_1,
	RANK_2,
	RANK_3,
	RANK_4,
	RANK_5,
	RANK_6,
	RANK_7,
	RANK_8,
	RANK_NB
};

enum CastlingRigths : int {
	CASTLE_NB = 0,
	WK = 1,
	WQ = 2,
	BK = 4,
	BQ = 8
};

constexpr Direction operator+(Direction d1, Direction d2) { return Direction(int(d1) + int(d2)); }
constexpr Direction operator*(int i, Direction d) { return Direction(i * int(d)); }

constexpr bool is_ok(Square s) { return s >= A1 && s <= H8; }
constexpr File file_of(Square s) { return File(s % 8); }
constexpr Rank rank_of(Square s) { return Rank(s >> 3); }
constexpr Square get_square(Rank rank, File file) { return static_cast<Square>(static_cast<int>(rank) * 8 + static_cast<int>(file)); }
constexpr Square get_square(int square_index) { return static_cast<Square>(square_index); }

// Additional operators to add a Direction to a Square
constexpr Square operator+(Square s, Direction d) { return Square(int(s) + int(d)); }
constexpr Square operator-(Square s, Direction d) { return Square(int(s) - int(d)); }
constexpr Square operator-(Square s, int d) { return Square(int(s) - d); }
inline Square& operator+=(Square& s, Direction d) { return s = s + d; }
inline Square& operator-=(Square& s, Direction d) { return s = s - d; }

// Additional operators to increment the ranks by some value
constexpr File operator+(File s, int d) { return File(int(s) + d); }
constexpr File operator-(File s, int d) { return File(int(s) - d); }

inline File& operator+=(File& f, int i) { return f = f + i; }

// Toggle the colour
constexpr Color operator~(Color c) { return Color(c ^ 1); }

constexpr Square make_square(File f, Rank r) { return Square((r << 3) + f); }

// bit macros
#define get_bit(bitboard, square) (bitboard & (1ULL << square)) // checks for available bit
#define set_bit(bitboard, square) (bitboard |= (1ULL << square)) // set piece to square
#define rm_bit(bitboard, square) ((bitboard) &= ~(1ULL << square)) // if theres a 1 remove it, if 0 don't

// Allow to use ++File, --File, ++Rank, --Rank and etc.
#define ENABLE_INCR_OPERATORS_ON(T) \
		inline T& operator++(T& d) { return d = T(int(d) + 1); } \
		inline T& operator--(T& d) { return d = T(int(d) - 1); } 

// Enables increment by one operation
ENABLE_INCR_OPERATORS_ON(Piece)
ENABLE_INCR_OPERATORS_ON(PieceType)
ENABLE_INCR_OPERATORS_ON(Square)
ENABLE_INCR_OPERATORS_ON(File)
ENABLE_INCR_OPERATORS_ON(Rank)

#undef ENABLE_INCR_OPERATORS_ON

#endif // !DEFS_H