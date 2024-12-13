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

	inline std::string move(Move m)
	{
		if (m == Move::invalid_move())
			return "(none)";

		if (m == Move::null_move())
			return "0000";

		Square source = m.source_square();
		Square target = m.target_square();

		if (m.move_type() == MT_CASTLING)
			target = make_square(target > source ? FILE_G : FILE_C, rank_of(source));

		std::string move_str = squareToCoordinates[source];
		move_str += squareToCoordinates[target];

		if (m.move_type() == MT_PROMOTION)
			move_str += ascii_pieces[get_piece(BLACK, m.promoted())];

		return move_str;
	}

	template<bool Root>
	U64 perft(Position& pos, int depth)
	{
		MoveInfo inf{};

		U64 count, nodes = 0;
		const bool leaf = (depth == 2);

		ScoredMove move_list[MAX_MOVES], * last;
		last = generate_moves(pos, move_list);

		for (const auto& m : MoveList(pos))
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
					? MoveList(pos).size()
					: perft<false>(pos, depth - 1);
				nodes += count;

				pos.undo_move(m, inf);
			}
			if(Root)
				std::cout << m << ": " << count << std::endl;
		}

		return nodes;
	}
}
#endif // !PERFT_H
