#include "bitboard.h"
#include "attacks.h"
#include "position.h"
#include "movegen.h"
#include "perft.h"
#include "move.h"
#include "uci.h"
#include "score.h"
#include "search.h"

#include <iostream>

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

		Move m{ E2, E4 };
		Move p{ m.encode(E4, E5, WHITE_PAWN, EMPTY, 0, 0, 0, 0) };

		std::cout << p.promoted() << "\n";
		std::cout << p.target_square() << "\n";


		//search_position(2);
	}
	else
		uci_loop();

	system("pause");
	return 0;
}