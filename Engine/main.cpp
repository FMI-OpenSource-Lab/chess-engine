#include <bitset>

#include "bitboard.h"
#include "position.h"
#include "movegen.h"
#include "perft.h"
#include "move.h"
#include "uci.h"
#include "score.h"
#include "search.h"

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
		pos.set("r3k2r/p1ppqpb1/bn2pnp1/1B1PN3/1p2P3/2N2Q1p/PPPB1PPP/R3K2R w KQkq - 0 1", &infos->back());

		Move m{ B5, C6 };

		std::cout << pos;
		std::cout << "\nGives check: " << (pos.gives_check(m) ? " yes" : " no");

		std::cout << "\n BLACK: " << (RANK_6 ^ 7) << "\n WHITE: " << (RANK_6);

		//search_position(2);
	}
	else
		//uci_loop();


		system("pause");
	return 0;
}