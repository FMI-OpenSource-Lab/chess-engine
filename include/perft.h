#pragma once

#include <iomanip>
#include <string>
#include <fstream>

#include "defs.h"
#include "position.h"
#include "move.h"
#include "movegen.h"

namespace KhaosChess
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

	inline U64 perft_driver(Position& pos, int depth)
	{
		if (depth == 0) return 1ULL;

		U64 nodes = 0;

		for (const auto& m : MoveList<GT_LEGAL>(pos))
		{
			MoveInfo move_info;
			pos.do_move(m, move_info);

			U64 count = perft_driver(pos, depth - 1);

			nodes += count;

			pos.undo_move(m);
		}

		return nodes;
	}

	inline void perft_debug(Position& pos, int depth)
	{
		U64 nodes = 0;
		U64 start_time = get_time_ms();

		for (const auto& m : MoveList<GT_LEGAL>(pos))
		{
			MoveInfo move_info;
			pos.do_move(m, move_info);

			U64 count = perft_driver(pos, depth - 1);

			pos.undo_move(m);

			std::cout << m.uci_move() << ": " << count << std::endl;

			nodes += count;
		}

		std::cout << "\nNodes: " << nodes;
		std::cout << "\nDepth: " << depth;
		std::cout << "\nTime: " << (get_time_ms() - start_time) << "ms\n";
	}

	inline void print_perft_table(Position& pos)
	{
		int depth;
		std::cout << "Depth: ";
		std::cin >> depth;

		std::cout << "\n\nDepth";
		std::cout << std::setw(20) << "Nodes";
		std::cout << std::setw(10) << "Time\n";
		std::cout << "--------------------------------------\n";

		for (int i = 0; i <= depth; i++)
		{
			U64 start_time = get_time_ms();
			
			std::cout << i << std::setw(24) << perft_driver(pos, i);
			std::cout << std::setw(10) << get_time_ms() - start_time << " ms\n";
		}
	}
}
