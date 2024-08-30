#include "bitboard.h"
#include "attacks.h"
#include "position.h"
#include "movegen.h"
#include "perft.h"
#include "move.h"

#include <iostream>
#include <string.h>
#include <bitset>

using namespace ChessEngine;

int get_time_ms()
{
#ifdef _WIN64
	return GetTickCount64();
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
	printf("\nNodes		Captures	E.p.	Castles		Promotions\n");
	printf("%ld		%ld		%ld	%ld		%ld\n", nodes, captures, ep, castles, promotions);
}

int main()
{
	init_all();

	system("pause");
	return 0;
}