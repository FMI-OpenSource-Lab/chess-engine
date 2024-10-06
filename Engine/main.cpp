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
		Position::init("4k3/1p4pp/2p5/1B6/q3r2Q/3p3P/1P4PK/4R3 w - - 0 1");

		Square ksq = getLS1B_square(bitboards[BLACK_KING]);

		Move m{ E4, C5 };

		// For inverted color

		U64 snipers = ( // pieces that are pinning
			(attacks_bb_by<ROOK>(ksq) &
				(bitboards[WHITE_QUEEN] |
					bitboards[BLACK_QUEEN] |
					bitboards[WHITE_ROOK] |
					bitboards[BLACK_ROOK]))
			|
			(attacks_bb_by<BISHOP>(ksq) &
				(bitboards[WHITE_QUEEN] |
					bitboards[BLACK_QUEEN] |
					bitboards[WHITE_BISHOP] |
					bitboards[BLACK_BISHOP]))
			) & occupancies[WHITE];

		U64 occ = occupancies[BOTH] ^ snipers; // everything without snipers
		U64 blocks{};
		U64 w_pinner{};

		while (snipers)
		{
			Square ssq = getLS1B_square(snipers); resetLSB(snipers);
			BITBOARD btw = in_between_bb(ksq, ssq) & occ; // blocker
			BITBOARD cut_ls1b = btw & (btw - 1);

			if (btw && !cut_ls1b) // make sure there is only 1 blocker to this attacker
				// if more than one blocker means the piece is not absolutely pinned
			{
				blocks |= btw; // append blocking (pinned) piece to blocks

				// get the white pinner piece
				if (btw & occupancies[WHITE])
					w_pinner |= ssq;
			}
		}

		std::cout << "Blocks bb: \n";
		print_bitboard(blocks);

		std::cout << "Pinner pieces for WHITE bb: \n";
		print_bitboard(w_pinner);

		//search_position(2);
	}
	else
		uci_loop();


	system("pause");
	return 0;
}