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

	std::string get_move_string(Move m)
	{
		if (m == Move::invalid_move())
			return "(none)";

		if (m == Move::null_move())
			return "0000";

		Square source = m.source_square();
		Square target = m.target_square();

		if (m.move_type() == MT_CASTLING)
			target = make_square(target > source ? FILE_G : FILE_C, rank_of(source));

		std::string soure_to_string = {
			char('a' + file_of(source)) , char('1' + rank_of(source))
		};
		std::string target_to_string = {
			char('a' + file_of(target)) , char('1' + rank_of(target))
		};

		std::string move = soure_to_string + target_to_string;

		if (m.move_type() == MT_PROMOTION)
			move += tolower(ascii_pieces[m.promoted()]);

		return move;
	}

	template<bool isRoot>
	uint64_t perft(Position& pos, int depth)
	{
		Info inf;

		uint64_t count, nodes = 0;
		bool leaf = (depth == 2);

		for (const auto& m: MoveList<GT_LEGAL>(pos))
		{
			if (isRoot && depth <= 1)
				count = 1, nodes++;
			else
			{
				pos.do_move(m, inf);
				count = leaf
					? MoveList<GT_LEGAL>(pos).size()
					: perft<false>(pos, depth - 1);

				nodes += count;

				pos.undo_move(m);
			}

			if (isRoot)
			{
				std::cout << get_move_string(m) << ": " << count << std::endl;

			}
		}

		return nodes;
	}

	inline uint64_t perft_driver(const char* fen_ptr, int depth)
	{
		InfoListPtr infos(new std::deque<Info>(1)); // double ended queue
		Position p;
		p.set(fen_ptr, &infos->back());

		return perft<true>(p, depth);
	}
}
#endif // !PERFT_H
