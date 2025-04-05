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

	bool debug = false;

	if (debug)
	{
		std::cout << "Debugging\n" << std::endl;

		Position pos{};
		InfoListPtr infos(new std::deque<MoveInfo>(1));

		// pin position: "3r4/bk6/8/r1pPK3/8/8/6B1/8 w - - 0 1";
		pos.set(TEST_FEN, &infos->back());
		
		std::cout << pos;

		print_perft_table(pos);

		// std::cout << "WHITE Evaluation: " << Eval::simple_evaluation(pos, WHITE) << "\n";
		// std::cout << "BLACK Evaluation: " << Eval::simple_evaluation(pos, BLACK) << "\n";
	}
	else
		uci_loop();

	system("pause");
	return 0;
}