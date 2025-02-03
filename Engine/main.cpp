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

		int depth = 5;
		std::string move;
		MoveInfo mi;

		pos.set(TEST_FEN, &infos->back());
		std::cout << pos << std::endl;;

		std::cout << perft<true>(pos, depth--);

		while (depth != 0)
		{
			std::cout << "\nEnter move: ";
			std::cin >> move;

			int source = (move[0] - 'a') + (8 - (move[1] - '0')) * 8;
			int target = (move[2] - 'a') + (8 - (move[3] - '0')) * 8;

			Move m{ Square(source), Square(target) };
			pos.do_move(m, mi);

			std::cout << perft(pos.get_fen().c_str(), depth--) << "\n";

			std::cout << pos.get_fen() << "\nPress any key to continue!";

			std::cin.get();
		}
	}
	else
		//uci_loop();

		system("pause");
	return 0;
}