#include "bboard.h"
#include "attacks.h"

#include <iostream>

int main()
{
	Attacks attacks;
	Bboard bb;
	
	// mask piece attacks at given square
	U64 attackMask = attacks.maskRookAttacks(a1);

	U64 occupancy = bb.set_occupancy(4095, bb.countBits(attackMask), attackMask);

	bb.print_bitboard(occupancy);

	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			int sq = i * 8 + j;

			std::cout << bb.countBits(attacks.maskRookAttacks(sq)) << ",";
		}
		std::cout << std::endl;
	}

	return 0;
}