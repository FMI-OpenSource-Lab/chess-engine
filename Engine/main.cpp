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
		
		pos.set("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/P1N2Q1p/1PPBBPPP/R3K2R b KQkq - 0 1");
		std::cout << pos << std::endl;

		/*print_bitboard(111);
		print_bitboard(238);

		std::cout << (pos.is_castling_interrupted(BK) ? "true" : "false") << "\n";
		std::cout << (pos.is_castling_interrupted(BQ) ? "true" : "false");*/

		ScoredMove ml[MAX_MOVES], * last;
		last = generate_moves(pos, ml);

		//std::cout << perft<true>(pos, 1);
	}
	else
		//uci_loop();

		system("pause");
	return 0;
}