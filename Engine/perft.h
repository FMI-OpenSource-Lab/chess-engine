#ifndef PERFT_H
#define PERFT_H

#include "defs.h"

namespace ChessEngine
{
	// leaf nodes
	// number of positions reached during the test of the move generator at given depth

	extern long nodes;
	extern long captures;
	extern long ep;
	extern long castles;
	extern long promotions;

	extern inline void perft_driver(int depth);
}
#endif // !PERFT_H
