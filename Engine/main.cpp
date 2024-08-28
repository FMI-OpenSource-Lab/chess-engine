#include "bitboard.h"
#include "attacks.h"
#include "position.h"
#include "move.h"

#include <iostream>
#include <string.h>

using namespace ChessEngine;

void init_all()
{
	// Initialize attacks
	Attacks::init();

	// Initialize bitboards
	Bitboards::init();

	// load the starting fen
	Position::init("r3k2r/p2pqpb1/bn2pnp1/2pPN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq c6 0 1 ");

	moves move_list[1];

	generate_moves(move_list);

	for (int move_count = 0; move_count < move_list->count; ++move_count)
	{
		int move = move_list->moves[move_count];

		// preserve 
		copy_board();

		make_move(move, MT_NORMAL);
		print_board();
		std::cin.get();

		restore_board();
		print_board();
		std::cin.get();
	}
}

int main()
{
	init_all();

	system("pause");
	return 0;
}