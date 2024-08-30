#include "perft.h"
#include "movegen.h"
#include "move.h"
#include "position.h"

namespace ChessEngine
{
	long nodes = 0;
	long captures = 0;
	long ep = 0;
	long castles = 0;
	long promotions = 0;

	inline void perft_driver(int depth)
	{
		if (depth == 0)
		{
			// inc nodes count
			nodes++;
			return;
		}

		moves move_list[1];

		generate_moves(move_list);

		for (int move_count = 0; move_count < move_list->count; ++move_count)
		{
			// preserve 
			copy_board();

			if (!make_move(move_list->moves[move_count], MT_NORMAL))
				// skip to the next move
				continue;

			// call perft driver
			perft_driver(depth - 1);

			restore_board();
		}
	}
}