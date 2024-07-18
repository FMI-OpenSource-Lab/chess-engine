#include<iostream>
#include "defs.h"
#include "consts.h"
#include "Attacks.h"

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

static inline int countBits(U64 bitboard)
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

// get LS1B
static inline int get_ls1b_index(U64 bitboard)
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

int main()
{
	Attacks attacks;

	U64 blockPieceBB = 0ULL;

	set_bit(blockPieceBB, d7);
	set_bit(blockPieceBB, g7);
	set_bit(blockPieceBB, e3);
	set_bit(blockPieceBB, b2);

	print_bitboard(blockPieceBB);
	
	printf("	index: %d	coordinate: %s\n", get_ls1b_index(blockPieceBB), squareToCoordinates[get_ls1b_index(blockPieceBB)]);
	//print_bitboard(attacks.generateRookAttacks(b3, blockPieceBB));

	U64 test = 0ULL;
	set_bit(test, get_ls1b_index(blockPieceBB));

	printf("index test: %d\n", get_ls1b_index(test));

	return 0;
}