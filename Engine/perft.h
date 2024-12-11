#ifndef PERFT_H
#define PERFT_H

#include <iomanip>
#include <string>

#include "defs.h"
#include "position.h"
#include "move.h"
#include "movegen.h"

namespace ChessEngine
{
	static U64 get_time_ms()
	{
#ifdef _WIN64
		return GetTickCount64();
#else
		struct timeval time_value;
		gettimeofday(&time_value, NULL);
		return time_value.tv_sec * 1000 + time_value.tv_usec / 1000;
#endif // _WIN64
	}

	extern std::string move(Move m);

	template<bool Root>
	U64 perft(Position& pos, int depth)
	{
		MoveInfo inf;

		U64 count, nodes = 0;
		const bool leaf = (depth == 2);

		for (const auto& m : MoveList<GT_LEGAL>(pos))
		{
			if (Root && depth <= 1)
			{
				count = 1;
				nodes++;
			}
			else
			{
				pos.do_move(m, inf);
				
				count = leaf
					? MoveList<GT_LEGAL>(pos).size()
					: perft<false>(pos, depth - 1);
				nodes += count;

				pos.undo_move(m, inf);
			}
			if (Root)
				std::cout << m;
		}

		return nodes;
	}
}
#endif // !PERFT_H
