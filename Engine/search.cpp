#include "search.h"

#include "attacks.h"
#include "score.h"
#include "move.h"
#include "movegen.h"
#include "perft.h"

#include <iostream>

namespace ChessEngine
{
	int ply = 0;
	int best_move = 0;

	int quiescence(int alpha, int beta)
	{
		// increment nodes
		nodes++;

		// evaluate pos
		int current_eval = evaluate();

		// fail hard beta proning
		if (current_eval >= beta)
			// node (move) fails high
			return beta;

		// better move
		if (current_eval > alpha)
		{
			// PV node (move)
			alpha = current_eval;
		}

		// create and generate moves
		moves move_list[1];
		generate_moves(move_list);

		// loop over the moves
		for (int mc = 0; mc < move_list->count; mc++)
		{
			// preserve state
			copy_board();

			// increase ply
			ply++;

			// only legal moves
			if (!make_move(move_list->moves[mc], MT_ONLY_CAPTURES))
			{
				ply--;
				continue; // skip to next move
			}

			// score current move
			int score = -quiescence(-beta, -alpha);

			// reduce the ply
			ply--;

			// undo move
			restore_board();

			// fail hard beta-cutoff
			if (score >= beta)
				// node (move) fails high
				return beta;

			// better move
			if (score > alpha)
			{
				// PV node (move)
				alpha = score;

			}
		}

		return alpha;
	}

	int negamax(int alpha, int beta, int depth)
	{
		// base case for recursion
		if (depth == 0)
			return quiescence(alpha, beta);

		// increment nodes
		nodes++;

		U64 king = !side ? bitboards[WHITE_KING] : bitboards[BLACK_KING];
		Square king_square = getLS1B_square(king);

		int in_check = is_square_attacked(king_square, ~side);
		int legal_moves = 0; // legal moves counter

		// to store the current best
		int current_best = 0;
		// to store the old alpha
		int old_alpha = alpha;

		// create and generate moves
		moves move_list[1];
		generate_moves(move_list);

		// loop over the moves
		for (int mc = 0; mc < move_list->count; mc++)
		{
			// preserve state
			copy_board();

			// increase ply
			ply++;

			// only legal moves
			if (!make_move(move_list->moves[mc], MT_NORMAL))
			{
				ply--;
				continue; // skip to next move
			}

			// inc legal moves counter
			legal_moves++;

			// score current move
			int score = -negamax(-beta, -alpha, depth - 1);

			// reduce the ply
			ply--;

			// undo move
			restore_board();

			// fail hard beta-cutoff
			if (score >= beta)
				// node (move) fails high
				return beta;

			// better move
			if (score > alpha)
			{
				// PV node (move)
				alpha = score;

				// if root
				if (!ply)
					// best move with the best score
					current_best = move_list->moves[mc];
			}
		}

		// dont have any legal moves in current position
		if (!legal_moves)
		{
			// 2 cases are present at this time
			// a: king is in check
			// b: king is not in check

			// a
			// in position with certain depth we can make for example two different checkmaates.
			// The short one and the longer one
			// If the both paths have the same score the engine cannot be sure which path to take since its the same for it.
			// by adding + ply value (which gets bigger the deeper it searches)
			// we ensure that the mate that requires less moves moves alrays has the biggest score, hence its prefered as the better path
			if (in_check) return -4900 + ply; // + ply is important
			return 0; // b -> stalemate score
		}

		// better move appeared
		if (old_alpha != alpha)
			// init best move
			best_move = current_best;

		// node (move) fails low
		return alpha;
	}

	void search_position(int depth)
	{
		// find best move within a given position
		int score = negamax(-50000, 50000, depth);

		if (best_move)
		{
			std::cout	<< "info score cp " << score
						<< " depth " << depth
						<< " nodes " << nodes << "\n";

			std::cout << "bestmove ";
			print_move(best_move);
			std::cout << "\n";
		}
	}
}