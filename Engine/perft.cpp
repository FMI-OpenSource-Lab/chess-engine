#include "perft.h"
#include "movegen.h"
#include "move.h"
#include "position.h"

#include<iostream>
#include <iomanip>

namespace ChessEngine
{
	long nodes = 0;

	int get_time_ms()
	{
#ifdef _WIN64
		return GetTickCount();
#else
		struct timeval time_value;
		gettimeofday(&time_value, NULL);
		return time_value.tv_sec * 1000 + time_value.tv_usec / 1000;
#endif // _WIN64
	}

	/*
	inline void perft_driver(int depth)
	{
		if (depth == 0)
		{
			// inc nodes count
			nodes++;
			return;
		}

		moves move_list[1];

		generate_moves(move_list);

		for (int move_count = 0; move_count < move_list->count; ++move_count)
		{
			// preserve 
			//copy_board();

			if (!make_move(move_list->moves[move_count], MT_NORMAL))
				// skip to the next move
				continue;

			// call perft driver
			perft_driver(depth - 1);

			//restore_board();
		}
	}

	void perft_test(int depth)
	{
		printf("\n     Performance test\n\n");

		// create move list instance
		moves move_list[1];

		// generate moves
		generate_moves(move_list);

		// init start time
		long start = get_time_ms();

		// loop over generated moves
		for (int move_count = 0; move_count < move_list->count; move_count++)
		{
			// preserve board state
			//copy_board();

			// make move
			if (!make_move(move_list->moves[move_count], MT_NORMAL))
				// skip to the next move
				continue;

			// cummulative nodes
			long cummulative_nodes = nodes;

			// call perft driver recursively
			perft_driver(depth - 1);

			// old nodes
			long old_nodes = nodes - cummulative_nodes;

			// take back
			//restore_board();

			//// print move
			//printf("%s%s%c :%ld\n",
			//	squareToCoordinates[get_move_source(move_list->moves[move_count])],
			//	squareToCoordinates[get_move_target(move_list->moves[move_count])],
			//	get_move_promoted(move_list->moves[move_count])
			//	? tolower(ascii_pieces[get_move_promoted(move_list->moves[move_count])])
			//	: ' ',
			//	old_nodes);
		}

		// print results
		printf("\n    Depth: %d\n", depth);
		printf("    Nodes: %ld\n", nodes);
		printf("     Time: %ld\n\n", get_time_ms() - start);
	}

	void print_perft_table()
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
			ULONGLONG start_time = get_time_ms();
			nodes = 0;

			perft_driver(i);

			std::cout << i << std::setw(24) << nodes << std::setw(10) << get_time_ms() - start_time << " ms\n";
		}
	}
	*/
}