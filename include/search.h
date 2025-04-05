#pragma once

#include "defs.h"

namespace KhaosChess
{
	extern int ply;
	extern int best_move;

	extern inline int negamax(int alpha, int beta, int depth);
	extern inline int quiescence(int alpha, int beta);
	extern int minimax(int alpha, int beta, int depth, bool maximizingPlayer);
	extern void search_position(int depth);
}
