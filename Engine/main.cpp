#include <iostream>

#include "position.h"
#include "bitboard.h"
#include "perft.h"
#include "vector_array.h"

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

		// pin position: "7B/2B5/3ppn2/3rk3/3b1q2/2Q2P2/7B/K2R4 w - - 0 1";

		pos.set("r3k2r/p2pqpb1/BnP1pnp1/4N3/4P3/5Q1p/PPPpKPPP/R6R w kq - 0 1");
		std::cout << pos << std::endl;

		MoveInfo mi{};

		// pos.do_move(Move{ D2, E1}, mi);
	}
	else
		//uci_loop();

		system("pause");
	return 0;
}