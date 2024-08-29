#include "bitboard.h"
#include "attacks.h"
#include "position.h"
#include "movegen.h"
#include "perft.h"

#include <iostream>
#include <string.h>

using namespace ChessEngine;

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

void init_all()
{
	// Initialize attacks
	Attacks::init();

	// Initialize bitboards
	Bitboards::init();

	// load the starting fen
	Position::init(TEST_FEN);

	// start time
	int start = get_time_ms();

	perft_driver(2);

	printf("Time taken to execute: %d ms\n", get_time_ms() - start);
	printf("Nodes: %ld\n", nodes);
}

int main()
{
	init_all();

	system("pause");
	return 0;
}