#ifndef PERFT_H
#define PERFT_H

#include "defs.h"
#include "position.h"

#include <iomanip>

namespace ChessEngine
{
	// leaf nodes
	// number of positions reached during the test of the move generator at given depth

	extern int get_time_ms();

	template<bool isRoot>
	uint64_t perft(Position& pos, int depth)
	{
		Info inf;

		uint64_t count, nodes = 0;
		const bool leaf = (depth == 2);


		return nodes;
	}
}
#endif // !PERFT_H
