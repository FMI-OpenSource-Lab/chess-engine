#ifndef PERFT_H
#define PERFT_H

#include "defs.h"

namespace ChessEngine
{
	// leaf nodes
	// number of positions reached during the test of the move generator at given depth

	extern long nodes;

	extern int get_time_ms();
	extern inline void perft_driver(int depth);
	extern void perft_test(int depth);
	extern void print_perft_table();
}
#endif // !PERFT_H
