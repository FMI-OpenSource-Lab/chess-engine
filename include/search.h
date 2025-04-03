#ifndef SEARCH_H
#define SEARCH_H

#include "defs.h"

namespace ChessEngine
{
	extern int ply;
	extern int best_move;

	extern inline int negamax(int alpha, int beta, int depth);
	extern inline int quiescence(int alpha, int beta);
	extern int minimax(int alpha, int beta, int depth, bool maximizingPlayer);
	extern void search_position(int depth);
}
#endif // !SEARCH_H
