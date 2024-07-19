#include "bboard.h"

#include<iostream>

Bboard::Bboard() {

}

void Bboard::print_bitboard(U64 bitboard)
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

U64 Bboard::set_occupancy(int index, int bitsInMask, U64 attackMask)
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