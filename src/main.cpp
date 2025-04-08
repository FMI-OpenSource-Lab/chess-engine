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
	Eval::init();

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
		std::cout << pos << std::endl;
		std::cout << "Game phase ((opening) 24 - 0 (endgame)): " << pos.game_phase() << std::endl;
		std::cout << "PST white: " << pos.pst_value(WHITE) << std::endl;
		std::cout << "PST black: " << pos.pst_value(BLACK) << std::endl;
	}
	else
		uci_loop();

	// system("pause");
	return 0;
}