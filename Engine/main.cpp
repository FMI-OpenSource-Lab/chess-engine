#include "bitboard.h"
#include "attacks.h"
#include "position.h"
#include "movegen.h"
#include "perft.h"
#include "move.h"
#include "uci.h"
#include "score.h"

#include <iostream>

using namespace ChessEngine;

void init_all()
{
	// Initialize attacks
	Attacks::init();

	// Initialize bitboards
	Bitboards::init();
}

int main()
{
	init_all();

	bool debug = true;

	if (debug)
	{
		std::cout << "Debugging\n\n";

		Position::init("rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 1");
		print_board();

		std::cout << evaluate() << "\n";
	}
	else
		uci_loop();


	system("pause");
	return 0;
}