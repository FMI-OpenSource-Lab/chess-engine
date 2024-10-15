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
	static int get_time_ms()
	{
#ifdef _WIN64
		return GetTickCount();
#else
		struct timeval time_value;
		gettimeofday(&time_value, NULL);
		return time_value.tv_sec * 1000 + time_value.tv_usec / 1000;
#endif // _WIN64
	}
	
	extern std::string move(Move m);
	extern void perft_test(Position& pos, int depth);
	extern void perft_driver(const char* fen, int depth);
}
#endif // !PERFT_H
