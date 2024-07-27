#include "bitboard.h"
#include "attacks.h"
#include "position.h"

#include <iostream>
#include <map>


void init_all()
{
	// Initialize attacks
	initAttacks();
	
	// Initialize bitboards
	Bitboards::init();

	// load the starting fen
	Position::init(START_FEN);
}

int main()
{
	init_all();

	system("pause");
	return 0;
}