// system headers
#ifndef DEFS_H
#define DEFS_H

#define NAME "iuli 2.1.0"

#if defined(_WIN64) && defined(_MSC_VER)  // No Makefile used
#include <intrin.h>                   // Microsoft header for _BitScanForward64()
#define IS_64BIT
#endif

#if defined(_MSC_VER)
	// Disable some silly and noisy warnings from MSVC compiler
#pragma warning(disable: 4127)  // Conditional expression is constant
#pragma warning(disable: 4146)  // Unary minus operator applied to unsigned type
#pragma warning(disable: 4800)  // Forcing value to bool 'true' or 'false'
#endif

// define bitboard data type
#define U64 unsigned long long

#ifdef _WIN64
#include<windows.h>
#else
#include<sys/time.h>
#endif

#ifdef IS_64BIT
constexpr bool Is64Bit = true;
#else 
constexpr bool Is64Bit = false;
#endif // IS_64BIT

#include <cassert>

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
	SQUARE_TOTAL = 64
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
	NO_PIECE,

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

	PIECE_NB = 14
};

enum Color
{
	WHITE,
	BLACK,
	BOTH
};

enum Direction : int {
	DOWN = 8,
	RIGHT = 1,
	UP = -DOWN,
	LEFT = -RIGHT,

	DOWN_LEFT = DOWN + LEFT,
	UP_LEFT = UP + LEFT,
	UP_RIGHT = UP + RIGHT,
	DOWN_RIGHT = DOWN + RIGHT
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
	RANK_8,
	RANK_7,
	RANK_6,
	RANK_5,
	RANK_4,
	RANK_3,
	RANK_2,
	RANK_1,
	RANK_NB
};

enum CastlingRights : int {
	CASTLE_NB = 0,
	WK = 1,
	WQ = 2,
	BK = 4,
	BQ = 8,

	KINGSIDE = WK | BK,
	QUEENSIDE = WQ | BQ,
	WHITE_CASTLE = WK | WQ,
	BLACK_CASTLE = BK | BQ,
	ANY = WHITE_CASTLE | BLACK_CASTLE,

	CASTLING_RIGHT_NB = 16
};

constexpr bool is_square_ok(Square s) { return s >= A8 && s <= H1; }

// Castling Rights operator overloads
inline CastlingRights operator&(Color c, CastlingRights cr) { return CastlingRights((c == WHITE ? WHITE_CASTLE : BLACK_CASTLE) & cr); }
inline CastlingRights operator|(CastlingRights& c, CastlingRights a) { return CastlingRights(int(c) | int(a)); };
inline CastlingRights operator&(CastlingRights& c, int d) { return CastlingRights(int(c) & int(d)); }

// Direction operator overloads
constexpr Direction operator+(Direction d1, Direction d2) { return Direction(int(d1) + int(d2)); }
constexpr Direction operator*(int i, Direction d) { return Direction(i * int(d)); }
constexpr Direction pawn_push_direction(Color c) { return c == WHITE ? UP : DOWN; }

// Additional operators to add a Direction to a Square
constexpr Square operator+(Square s, Direction d) { return Square(int(s) + int(d)); }
constexpr Square operator-(Square s, Direction d) { return Square(int(s) - int(d)); }
constexpr Square operator-(Square s, int d) { return Square(int(s) - d); }
constexpr Square operator+(Square s, int d) { return Square(int(s) + d); }
inline Square& operator+=(Square& s, Direction d) { return s = s + d; }
inline Square& operator-=(Square& s, Direction d) { return s = s - d; }

inline Square operator|(Square s1, Square s2) { return Square(int(s1) | int(s2)); }
inline Square operator|(Square s1, int n) { return Square(int(s1) | n); }

inline bool operator==(Square s1, Square s2) { return int(s1) == int(s2); }

// Additional operators to increment the ranks by some value
constexpr File operator+(File s, int d) { return File(int(s) + d); }
constexpr File operator-(File s, int d) { return File(int(s) - d); }

inline File& operator+=(File& f, int i) { return f = f + i; }

// Toggle the colour
constexpr Color operator~(Color c) { return Color(c ^ 1); }

// Toggle the piece (WHITE_PAWN to BLACK_PAWN)
constexpr Piece operator~(Piece p) { return Piece(p ^ 8); }

// Rank and File operator overloads
constexpr File file_of(Square s) { return File(s % 8); }
constexpr Rank rank_of(Square s) { return Rank(s >> 3); }
constexpr Rank rank_relative_to_side(Color c, Rank r) { return Rank(r ^ (c * 7)); }

// Square helper methods
constexpr Square convert_to_square(int rank, int file) { return Square(rank * 8 + file); }
constexpr Square convert_to_square(Rank rank, File file) { return convert_to_square(int(rank), int(file)); }
constexpr Square make_square(File f, Rank r) { return Square((r << 3) + f); }
constexpr Square sq_relative_to_side(Square s, Color c) { return Square(int(s) ^ (c * 56)); }

// Piece, PieceType and Color helper methods
constexpr Piece get_piece(Color c, PieceType pt) { return Piece(pt + (c * 6)); }

constexpr PieceType type_of_piece(Piece p) { return PieceType(p - 6 * int(p / 7)); }

constexpr Color get_piece_color(Piece p)
{
	assert(p != NO_PIECE);
	// if piece is below 7 (white) the number will be betwenn [0:1)
	// will become 1 or greater if piece is black
	// due to integer rounding we work with 0 and 1 only
	return Color(p / 7);
}

constexpr int TOTAL_MAX_DEPTH = 512;

typedef unsigned short PLY_TYPE; // 16 bit 

constexpr int MAX_MOVES = 256;
constexpr int MAX_PLY = 246;

typedef int Value; // 32 bit

constexpr Value VALUE_ZERO = 0;
constexpr Value VALUE_DRAW = 0;
constexpr Value VALUE_NONE = 32002;
constexpr Value VALUE_INFINITE = 32001;

constexpr Value VALUE_MATE = 32001;
constexpr Value VALUE_MATE_IN_MAX_PLY = VALUE_MATE - MAX_PLY;

// Piece values estimated by AlphaZero
constexpr Value PAWN_VALUE = 100;
constexpr Value KNIGHT_VALUE = 305;
constexpr Value BISHOP_VALUE = 333;
constexpr Value ROOK_VALUE = 563;
constexpr Value QUEEN_VALUE = 950;

constexpr Value PieceValue[PIECE_NB] // All pieces 
{
	// WHITE
	VALUE_ZERO, PAWN_VALUE, KNIGHT_VALUE, BISHOP_VALUE, ROOK_VALUE, QUEEN_VALUE, VALUE_ZERO,
	// BLACK
				PAWN_VALUE, KNIGHT_VALUE, BISHOP_VALUE, ROOK_VALUE, QUEEN_VALUE, VALUE_ZERO, VALUE_ZERO
};

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

// namespace ChessEngine

#endif // !DEFS_H