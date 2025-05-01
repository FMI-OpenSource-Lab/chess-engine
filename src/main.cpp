#include <iostream>
#include <numeric>

#include "position.h"
#include "bitboard.h"
#include "perft.h"
#include "uci.h"
#include "score.h"
#include "endgame.h"

using namespace KhaosChess;

int main()
{
	Bitboards::init();
	BitBase::init();
	Endgames::init();

	bool debug = true;

	if (debug)
	{
		std::cout << "Debugging\n"
				  << std::endl;

		Position pos{};
		InfoListPtr infos(new std::deque<MoveInfo>(1));

		// pin position: "3r4/bk6/8/r1pPK3/8/8/6B1/8 w - - 0 1";
		// pos.set(TEST_FEN, &infos->back());

		std::string kq_vs_kr = "8/3kr3/8/8/3K1Q2/8/8/8 w - - 0 1";

		pos.set(kq_vs_kr, &infos->back());

		Scorer<SC_ALL>().print_stats(pos);
	}
	else
		uci_loop();

	// system("pause");
	return 0;
}