#pragma once
#ifndef BITBOARD_H
#define BITBOARD_H

#include <cassert>
#include <cstdint>
#include <iostream>
#include <array>

#include "consts.h"

// Declare prototypes
constexpr inline U64 set_occupancy(int index, int bitsInMask, U64 attackMask);

extern constexpr int getLS1B(U64 bitboard);
extern constexpr int countBits(U64 bitboard);

extern void init_sliders_attacks(PieceType py);
extern U64 bishopAttacks(U64 occ, Square sq);
extern U64 rookAttacks(U64 occ, Square sq);

constexpr U64 squareBB(Square square)
{
	assert(is_ok(square));
	return (1ULL << square);
}

namespace Bitboards
{
	void init();
};

// overloads of bitwise operators between bitboard and square for testing purposes
inline U64 operator&(U64 b, Square s) { return b & squareBB(s); }
inline U64 operator|(U64 b, Square s) { return b | squareBB(s); }
inline U64 operator^(U64 b, Square s) { return b ^ squareBB(s); }
inline U64& operator|=(U64 b, Square s) { return b |= squareBB(s); }
inline U64& operator^=(U64 b, Square s) { return b ^= squareBB(s); }

inline U64 operator&(Square s, U64 b) { return b & s; }
inline U64 operator|(Square s, U64 b) { return b | s; }
inline U64 operator^(Square s, U64 b) { return b ^ s; }

inline U64 operator|(Square s1, Square s2) { return squareBB(s1) | s2; }

// Files
constexpr U64 FileA_Bits = 0x0101010101010101ULL; // first row is ones
constexpr U64 FileB_Bits = FileA_Bits << 1;
constexpr U64 FileC_Bits = FileA_Bits << 2;
constexpr U64 FileD_Bits = FileA_Bits << 3;
constexpr U64 FileE_Bits = FileA_Bits << 4;
constexpr U64 FileF_Bits = FileA_Bits << 5;
constexpr U64 FileG_Bits = FileA_Bits << 6;
constexpr U64 FileH_Bits = 0x8080808080808080ULL;

// Ranks, displayed look flipped
constexpr U64 Rank1_Bits = 0xFF;
constexpr U64 Rank2_Bits = Rank1_Bits << (8 * 1);
constexpr U64 Rank3_Bits = Rank1_Bits << (8 * 2);
constexpr U64 Rank4_Bits = Rank1_Bits << (8 * 3);
constexpr U64 Rank5_Bits = Rank1_Bits << (8 * 4);
constexpr U64 Rank6_Bits = Rank1_Bits << (8 * 5);
constexpr U64 Rank7_Bits = Rank1_Bits << (8 * 6);
constexpr U64 Rank8_Bits = 0x8080808080808080;

// displayed looks flipped
constexpr U64 Diagonal_A1_H8 = 0x8040201008040201ULL;
constexpr U64 Diagonal_H1_A8 = 0x0102040810204080ULL;

constexpr U64 LightSquares = 0x55AA55AA55AA55AAULL;
constexpr U64 DarkSquares = 0xAA55AA55AA55AA55ULL;

// Not files
constexpr U64 not_A = 18374403900871474942ULL;	// ~FileA_Bits bitboard value where the A file is set to zero
constexpr U64 not_H = 9187201950435737471ULL;	// ~FileH_Bits bitboard value where the H file is set to zero
constexpr U64 not_HG = 4557430888798830399ULL;	// ~FileH_Bits & ~FileG_Bits bitboard value where the HG files are set to zero
constexpr U64 not_AB = 18229723555195321596ULL;	// ~FileA_Bits & ~FileB_Bits bitboard value where the HG files are set to zero


// define pawn attacks table [side][square]
static U64 pawnAttacks[2][64]; // 2 - sides to play, 64 - squares on a table

// define knight attacks table [square]
static U64 knightAttacks[64];

// define king attack table [square]
static U64 kingAttacks[64];

// define magic bishop attack table [squares][occupancy]
static U64 mBishopAttacks[64][512]; // 256 K

// define magic rook attack table [squares][occupancy]
static U64 mRookAttacks[64][4096]; // 2048K

struct SMagic {
	U64 mask;  // to mask relevant squares of both lines (no outer squares)
	U64 magic; // magic 64-bit factor
};

inline void print_bitboard(U64 bitboard)
{
	std::cout << std::endl;

	// loop on ranks
	for (unsigned short rank = 0; rank < 8; rank++)
	{
		// loop on files
		for (unsigned short file = 0; file < 8; file++)
		{
			// convert rank and file into square index
			int sq = rank * 8 + file;

			if (!file) printf("  %d ", 8 - rank);

			// print bit state (1 or 0)
			printf(" %d", get_bit(bitboard, sq) ? 1 : 0);
		}

		std::cout << std::endl;
	}

	// print files
	std::cout << "\n     a b c d e f g h \n\n";

	// print as udec value
	std::cout << "     Bitboard: " << bitboard << "\n\n";
}

constexpr inline U64 set_occupancy(int index, int bitsInMask, U64 attackMask)
{
	// occupancy map
	U64 occupancy = 0ULL;

	// loop on bits within attack mask
	for (int count = 0; count < bitsInMask; count++)
	{
		// get the lsb square
		int square = getLS1B(attackMask);

		// remove LSB
		rm_bit(attackMask, square);

		// make occupancy on board
		if (index & (1 << count))
			// populate occupancy map
			occupancy |= (1ULL << square);
	}

	return occupancy;
}

extern constexpr int getLS1B(U64 bitboard)
{
	// check if bb is not 0
	if (bitboard)
	{
		// count trailing bits before LS1B
		U64 twosComplement = ~bitboard + 1; // -bitboard is equivelent
		return countBits((bitboard & twosComplement) - 1);
	}
	else
		return -1;
}

extern constexpr int countBits(U64 bitboard)
{
	// init variable for count
	int count = 0;

	// loop unltill board is empty
	while (bitboard)
	{
		// increment
		count++;

		// reset LSB
		bitboard &= bitboard - 1;
	}

	return count;
}

#endif // !BITBOARD_H
