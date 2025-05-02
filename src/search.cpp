#include "search_engine.h"

#include "movegen.h"
#include "score.h"

#include <iostream>
#include <algorithm>

namespace KhaosChess
{
	// Visibility is only for this .cpp file
	static PLY_TYPE ply = 0;
	static Move best_move = NO_MOVE;
	static uint64_t nodes = 0;

	inline Value quiescence(Value alpha, Value beta, Position &pos)
	{
		nodes++;

		Value current_score = Scorer<SC_ALL>().get_score(pos);

		// fail hard beta pruning
		if (current_score >= beta)
			return beta;

		if (current_score > alpha)
			alpha = current_score;

		for (const auto &m : MoveList<GT_LEGAL>(pos))
		{
			MoveInfo move_info;
			pos.do_move(m, move_info);

			ply++;

			Value score = -quiescence(-beta, -alpha, pos);

			ply--;

			pos.undo_move(m);

			if (score >= beta)
				return beta;

			if (score > alpha)
				alpha = score;
		}

		return alpha;
	}

	inline Value negamax(Value alpha, Value beta, int depth, Position &pos)
	{
		if (depth == 0)
			return quiescence(alpha, beta, pos);

		nodes++;

		Color c = pos.side_to_move();
		Square ksq = pos.square<KING>(c);
		bool is_in_check = pos.is_square_attacked(ksq, ~c);

		if (is_in_check)
			depth++;

		Move current_best = NO_MOVE;
		Value old_alpha = alpha;

		for (const auto &m : MoveList<GT_LEGAL>(pos))
		{
			MoveInfo move_info;
			pos.do_move(m, move_info);

			ply++;

			Value score = -negamax(-beta, -alpha, depth - 1, pos);

			ply--;

			pos.undo_move(m);

			if (score >= beta)
				return beta;

			if (score > alpha)
			{
				alpha = score;

				if (ply == 0)
					current_best = m;
			}
		}

		// No legal moves
		if (MoveList<GT_LEGAL>(pos).size() == 0)
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

			return (-4900 + ply) * is_in_check;
		}

		if (old_alpha != alpha)
			best_move = current_best;

		return alpha;
	}

	void search_position(int depth, Position &pos)
	{
		Color side = pos.side_to_move();
		std::cout << "Move: " << best_move << "\n";

		Value score = negamax(-VALUE_INFINITE, VALUE_INFINITE, depth, pos);

		std::cout << "Negamax: " << score << "\n";

		if (best_move != NO_MOVE)
		{
			std::cout << "info score cp " << score
					  << " depth " << depth
					  << " nodes " << nodes
					  << " side " << (side == WHITE ? "WHITE" : "BLACK") << "\n";

			std::cout << "bestmove " << best_move << "\n";
		}
	}

}