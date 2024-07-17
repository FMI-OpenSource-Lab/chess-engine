#include<iostream>
#include "defs.h"
#include "consts.h"
#include "attacks.cpp"

void print_bitboard(U64 bitboard)
{
	std::cout << "\n";

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

		printf("\n");
	}

	// print files
	std::cout << "\n     a b c d e f g h \n\n";

	// print as udec value
	printf("     Bitboard: %llud\n\n", bitboard);
}

int main()
{
	attacks attacks;
	print_bitboard(attacks.maskKnightAttacks(e4));

	return 0;
}