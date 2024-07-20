#pragma once
#ifndef BITBOARD_H
#define BITBOARD_H

#include "defs.h"
#include <string>
#include <cassert>
#include <iostream>

// Declare prototypes
constexpr inline U64 set_occupancy(int index, int bitsInMask, U64 attackMask);
extern constexpr int getLS1B(U64 bitboard);
extern constexpr int countBits(U64 bitboard);

constexpr U64 squareBB(Square square)
{
	assert(is_ok(square));
	return (1ULL << square);
}

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
