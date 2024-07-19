#pragma once
#ifndef BBOARD_H
#define BBOARD_H

#include "defs.h"

class Bboard 
{
public:
	Bboard ();
	
	void print_bitboard(U64 bitboard);

	U64 set_occupancy(int index, int bitsInMask, U64 attackMask);
	
	constexpr inline int getLS1B(U64 bitboard)
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

	constexpr inline int countBits(U64 bitboard)
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
};

#endif // !BITBOARD_H
