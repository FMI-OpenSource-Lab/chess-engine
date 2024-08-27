#include "bitboard.h"
#include "attacks.h"
#include "position.h"
#include "move.h"

#include <iostream>

using namespace ChessEngine;

void init_all()
{
	// Initialize attacks
	Attacks::init();

	// Initialize bitboards
	Bitboards::init();

	// load the starting fen
	Position::init(TEST_FEN);

	moves move_list[1];

	generate_moves(move_list);
	
	print_move_list(move_list);
}

int main()
{
	init_all();

	system("pause");
	return 0;
}