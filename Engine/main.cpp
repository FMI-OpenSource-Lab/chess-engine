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

	Color c = WHITE;

	// load the starting fen
	//Position::init(TEST_ATTACKS_FEN, c);

	//for (size_t i = 0; i < 64; i++)
	//{
	//	print_bitboard(pawn_attacks_mask(BLACK, (Square)i));
	//}
}

int main()
{
	init_all();
	
	std::cout << "AND Operation bb: ";
	print_bitboard(1310720ULL & 134217728ULL);
	
	std::cout << "black pawn attack bb: ";
	print_bitboard(1310720ULL);

	std::cout << "white pawn bb: ";
	print_bitboard(134217728ULL);

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