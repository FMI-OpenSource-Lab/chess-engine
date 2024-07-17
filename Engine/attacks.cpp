#include <iostream>

#include "defs.h"
#include "consts.h"

class attacks
{
public:
	// define pawn attacks table [side][square]
	U64 pawnAttacks[2][64]; // 2 - sides to play, 64 - squares on a table

	// define knight attacks table [square]
	U64 knightAttacks[64];

	U64 maskPawnAttacks(int side, int square)
	{
		// result attacks
		U64 attacks = 0ULL;

		// piece bitboard
		U64 bitboard = 0ULL;

		// set piece on board
		set_bit(bitboard, square);

		// white
		if (!side)
		{
			// generate attacks
			if ((bitboard >> 7) & notAFile) attacks |= (bitboard >> 7);
			if ((bitboard >> 9) & notHFile) attacks |= (bitboard >> 9);
		}
		else // black
		{
			// generate attacks
			if ((bitboard << 7) & notHFile) attacks |= (bitboard << 7);
			if ((bitboard << 9) & notAFile) attacks |= (bitboard << 9);
		}

		return attacks;
	}

	U64 maskKnightAttacks(int square)
	{
		// result attacks
		U64 attacks = 0ULL;

		// piece bitboard
		U64 bitboard = 0ULL;

		// set piece on board
		set_bit(bitboard, square);

		// generate attacks
		if ((bitboard >> 17) & notHFile)  attacks |= (bitboard >> 17); // up and right
		if ((bitboard >> 15) & notAFile)  attacks |= (bitboard >> 15); // up and left
		if ((bitboard >> 10) & notHGFile) attacks |= (bitboard >> 10); // right
		if ((bitboard >> 6) & notABFile)  attacks |= (bitboard >> 6);  // left

		// flip the offset
		if ((bitboard << 17) & notAFile)  attacks |= (bitboard << 17); // down and right
		if ((bitboard << 15) & notHFile)  attacks |= (bitboard << 15); // down and left
		if ((bitboard << 10) & notABFile) attacks |= (bitboard << 10); // right
		if ((bitboard << 6) & notHGFile)  attacks |= (bitboard << 6);  // left

		return attacks;
	}

	void initAttacks()
	{
		for (int sq = 0; sq < 64; sq++)
		{
			pawnAttacks[white][sq] = maskPawnAttacks(white, sq);
			pawnAttacks[black][sq] = maskPawnAttacks(black, sq);

			knightAttacks[sq] = maskKnightAttacks(sq);
		}
	}
};