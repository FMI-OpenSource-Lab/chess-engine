#include <iostream>

#include "position.h"
#include "bitboard.h"
#include "perft.h"
#include "vector_array.h"
#include "uci.h"

using namespace ChessEngine;

int main()
{
	Bitboards::init();

	bool debug = true;

	if (debug)
	{
		std::cout << "Debugging\n" << std::endl;

		Position pos{};
		InfoListPtr infos = InfoListPtr(new std::deque<MoveInfo>(1));

		// pin position: "3r4/bk6/8/r1pPK3/8/8/6B1/8 w - - 0 1";
		pos.set(TEST_FEN, &infos->back());
		std::cout << pos;

		std::cout << "Nodes: " << perft<true>(pos, 5);
	}
	else
		uci_loop();

	system("pause");
	return 0;
}