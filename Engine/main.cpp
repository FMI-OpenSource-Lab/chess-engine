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
		InfoListPtr infos(new std::deque<MoveInfo>(1));

		// pin position: "3r4/bk6/8/r1pPK3/8/8/6B1/8 w - - 0 1";
		pos.set("r3k3/p1ppqpb1/Bn2pnp1/3PN3/1p2r3/2N2Q1p/PPPB1PPP/1R2K2R w Kq - 0 3", &infos->back());
		
		std::cout << pos;

		perft_root(pos, 1);
	}
	else
		uci_loop();

	system("pause");
	return 0;
}