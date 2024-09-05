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

		Position::init(TEST_FEN);

		moves move_list[1];
		generate_moves(move_list);

		for (int c = 0; c <= move_list->count; c++)
		{
			score_move(move_list->moves[c]);
		}
	}
	else
		uci_loop();

	system("pause");
	return 0;
}