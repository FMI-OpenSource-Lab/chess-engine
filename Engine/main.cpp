#include <iostream>

#include "position.h"
#include "bitboard.h"
#include "perft.h"

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
		InfoListPtr infos = InfoListPtr(new std::deque<Info>(1));
		pos.set(TEST_FEN, &infos->back());
		std::cout << pos;

		Info inf;

		pos.do_move(Move(E2, D1), inf);
	
		std::cout << pos;

		pos.do_move(Move(C7, C5), inf);
		std::cout << pos;

		pos.do_move(Move(E1, E2), inf);
		std::cout << pos;

		//perft_test(pos, 1);
		//search_position(2);
	}
	else
		//uci_loop();

		system("pause");
	return 0;
}