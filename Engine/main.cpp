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

		Position pos;
		InfoListPtr infos = InfoListPtr(new std::deque<MoveInfo>(1));
		
		const char* pin_pos = "7B/2B5/3ppn2/3rk3/3b1q2/2Q2P2/7B/K2R4 w - - 0 1";

		pos.set(pin_pos, &infos->back());
		std::cout << pos;
		print_bitboard(pos.get_pinned_pieces(WHITE));

		//Info inf;

		//pos.do_move(Move(E2, D1), inf);

		//std::cout << pos;

		//pos.do_move(Move(C7, C5), inf);
		//std::cout << pos;

		//pos.do_move(Move(E1, E2), inf);
		//std::cout << pos;

		//perft_test(pos, 1);
		//search_position(2);
	}
	else
		//uci_loop();

		system("pause");
	return 0;
}