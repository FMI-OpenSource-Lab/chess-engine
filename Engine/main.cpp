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
		Position::init("4k3/1p4pp/2p5/2N5/q6Q/3p3P/1P4PK/4R3 w - - 0 1");

		Square ksq = getLS1B_square(bitboards[BLACK_KING]);
		Square rsq = getLS1B_square(bitboards[WHITE_ROOK]);

		Move m{ E4, C5 };

		// Discovered check
		//	Determine the direction between the source square and the king square
		//	If both squares share a common line:
		//  call an appropriate sliding ray or line attack getter
		//  with king square and occ to look whether this SET intersects 
		//  the possible opponing sliders of that ray.

		U64 btw_ksq_src = in_between_bb(ksq, m.source_square());
		
		U64 snipers = (
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
			) & occupancies[side];

		U64 occ = bitboards[BOTH] ^ snipers;



		print_bitboard(snipers);

		//search_position(2);
	}
	else
		uci_loop();


	system("pause");
	return 0;
}