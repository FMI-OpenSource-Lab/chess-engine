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
		_Position pos;
		InfoListPtr infos = InfoListPtr(new std::deque<Info>(1));
		pos._set(START_FEN, &infos->back());

		std::cout << pos;

		//search_position(2);
	}
	else
		//uci_loop();


	system("pause");
	return 0;
}