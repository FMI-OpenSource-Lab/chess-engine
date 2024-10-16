#include "perft.h"

namespace ChessEngine
{
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

		std::string move = squareToCoordinates[source];
		move += squareToCoordinates[target];

		if (m.move_type() == MT_PROMOTION)
			move += tolower(ascii_pieces[m.promoted()]);

		return move;
	}

	void perft_test(Position& pos, int depth)
	{
		uint64_t nodes = 0;
		Info inf{};

		long start = get_time_ms();

		for (const auto& m : MoveList<GT_LEGAL>(pos))
		{
			if (depth < 1)
			{
				nodes++;
				return;
			}
			else
			{
				pos.do_move(m, inf);

				uint64_t comulative_nodes = nodes;

				perft_test(pos, depth - 1);

				uint64_t old_nodes = nodes - comulative_nodes;

				pos.undo_move(m);

				std::cout << move(m) << ": " << old_nodes << std::endl;
				std::cout << "\nDepth: " << depth
					<< "\nNodes: " << nodes
					<< "\nTime: " << get_time_ms() - start
					<< std::endl;
			}
		}
	}

	void perft_driver(const char* fen_ptr, int depth)
	{
		// double ended queue
		InfoListPtr infos = InfoListPtr(new std::deque<Info>(1));
		Position p{};

		p.set(fen_ptr, &infos->back());

		perft_test(p, depth);
	}
}