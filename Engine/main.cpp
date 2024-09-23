﻿#include <bitset>

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
		_Position p;
		Info *inf{};

		p.set(START_FEN, inf);
		print_board();

		//search_position(2);
	}
	else
		uci_loop();


	system("pause");
	return 0;
}