#include <iostream>

#include "position.h"
#include "bitboard.h"
#include "perft.h"
#include "deque.h"

using namespace ChessEngine;

void init_all()
{
	// Initialize attacks
	// Attacks::init();

	// Initialize bitboards
	Bitboards::init();
}

int main()
{
	init_all();
	bool debug = true;

	if (debug)
	{
		std::cout << "Debugging\n" << std::endl;

		Position pos{};
		InfoListPtr infos = InfoListPtr(new std::deque<MoveInfo>(1));

		const char* pin_pos = "7B/2B5/3ppn2/3rk3/3b1q2/2Q2P2/7B/K2R4 w - - 0 1";

		pos.set("r3k2r/p1ppqpb1/bnB1pnp1/3PN3/1p2P3/2N2Q1p/PPPB1PPP/R3K2R w KQkq - 0 1");
		std::cout << pos << std::endl;

		Move check_move = Move::make<MT_NORMAL>(C6, B7, BISHOP);
		Piece p = pos.get_piece_on(check_move.source_square());

		// Test if get piece on works
		std::cout << p << "\n";

		// Test if gives check works
		std::cout << "Is giving check?\n" 
			<< (pos.gives_check(check_move) ? "True" : "False") << "\n";

		// Test if place and remove piece works
		pos.place_piece(p, check_move.target_square());
		pos.remove_piece(check_move.source_square());

		std::cout << pos;
	}
	else
		//uci_loop();

		system("pause");
	return 0;
}