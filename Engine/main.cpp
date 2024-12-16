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

		// pin position: "3r4/bk6/8/r1pPK3/8/8/6B1/8 w - - 0 1";

		pos.set("r3k2r/p1ppqpb1/bnN1pnp1/3P4/1p2P3/2N2Q1p/PPPBBPPP/R3K2R b KQkq - 1 1");
		std::cout << pos << std::endl;

		std::cout << "Nodes: " << perft<true>(pos, 1);
	}
	else
		//uci_loop();

		system("pause");
	return 0;
}