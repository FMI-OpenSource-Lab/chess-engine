#include "move.h"
#include "consts.h"
#include "position.h"
#include "attacks.h"
#include "bitboard.h"

#include <stdio.h>

namespace ChessEngine
{
	// Get all pawn attacks on the respective bits on the board
	inline U64 all_board_pawn_attacks(U64 wattacks[], U64 wpawns)
	{
		U64 pawn_attack_bb = 0ULL;

		while (wpawns)
		{
			int source = getLS1B(wpawns);
			pawn_attack_bb |= wattacks[source] | wpawns;
			resetLSB(wpawns);
		}

		return pawn_attack_bb;
	}

	inline U64 white_single_push_target(U64 wpawns, U64 empty) 
	{
		return up_one(wpawns) & empty; 
	}

	inline U64 black_single_push_target(U64 bpawns, U64 empty) 
	{ 
		return down_one(bpawns) & empty;
	}

	inline U64 white_double_push_target(U64 wpawns, U64 empty)
	{
		U64 singlePushs = white_single_push_target(wpawns, empty);
		return up_one(singlePushs) & empty & Rank4_Bits;
	}

	inline U64 black_double_push_target(U64 bpawns, U64 empty)
	{
		U64 singlePushs = black_single_push_target(bpawns, empty);
		return down_one(singlePushs) & empty & Rank5_Bits;
	}
}