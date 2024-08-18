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
	Position::init("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPpP/R3K2R b KQkq - 0 1 ");

	generate_moves();
}

int main()
{
	init_all();

	system("pause");
	return 0;
}