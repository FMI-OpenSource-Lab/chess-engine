#include "bitboard.h"
#include "attacks.h"
#include "position.h"

#include <iostream>

using namespace ChessEngine;

void init_all()
{
	// Initialize attacks
	Attacks::init();

	// Initialize bitboards
	Bitboards::init();

	// load the starting fen
	Position::init(TEST_ATTACKS_FEN, WHITE);
}

int main()
{
	init_all();

	// print_bitboard(knight_attacks_mask(E5));

	/*if (2251799813685248)
	{
		U64 da = (11258999068426240ULL & 2251799947902976ULL);

		std::cout << "AND Operation: ";
		std::cout << da << "\n\n\n";

		std::cout << "AND Operation bb: ";
		print_bitboard(da);

		std::cout << "Pawn attack bb: ";
		print_bitboard(11258999068426240ULL);

		std::cout << "BBS bb: ";
		print_bitboard(2251799947902976ULL);
	}*/

	system("pause");
	return 0;
}