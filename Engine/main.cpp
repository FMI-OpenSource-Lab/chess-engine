#include <bitset>

#include "bitboard.h"
#include "attacks.h"
#include "position.h"
#include "movegen.h"
#include "perft.h"
#include "move.h"
#include "uci.h"
#include "score.h"
#include "search.h"

using namespace ChessEngine;

void init_all()
{
	// Initialize attacks
	Attacks::init();

	// Initialize bitboards
	Bitboards::init();
}

int main()
{
	init_all();
	bool debug = true;

	if (debug)
	{
		std::cout << "Debugging\n\n";

		Position::init(START_FEN);
		print_board();

		print_bitboard(rookAttacks(0ULL, E4));
		print_bitboard(siding_attacks(ROOK, E4, 0ULL));
		
		//search_position(2);
	}
	else
		uci_loop();

	system("pause");
	return 0;
}