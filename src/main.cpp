#include <iostream>

#include "position.h"
#include "bitboard.h"
#include "perft.h"
#include "uci.h"
#include "score.h"

using namespace KhaosChess;

int main()
{
	Bitboards::init();

	bool debug = true;

	if (debug)
	{
		std::cout << "Debugging\n"
				  << std::endl;

		Position pos{};
		InfoListPtr infos(new std::deque<MoveInfo>(1));

		// pin position: "3r4/bk6/8/r1pPK3/8/8/6B1/8 w - - 0 1";
		// pos.set(TEST_FEN, &infos->back());

		// std::cout << pos;

		pos.set("r3k3/p1pp1p2/8/8/8/8/PPP2PPP/3RK2R w Kq - 0 1", &infos->back());
	}
	else
		uci_loop();

	// system("pause");
	return 0;
}