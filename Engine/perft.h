#ifndef PERFT_H
#define PERFT_H

#include <iomanip>
#include <string>
#include <fstream>

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

	inline std::string uci_move(Move m)
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

		std::ofstream perft_results;
		perft_results.open("../results.txt");

		U64 start = get_time_ms();

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

				// std::cout << pos << "\n";

				pos.undo_move(m, inf);
			}
			if (Root)
			{
				std::cout << uci_move(m) << ": " << count << std::endl;
				perft_results << uci_move(m) << ": " << count << std::endl;
			}
		}
		perft_results.close();

		std::cout << "\nDepth: " << depth;
		std::cout << "\nTime: " << (get_time_ms() - start) << "ms\n";

		return nodes;
	}
}
#endif // !PERFT_H
