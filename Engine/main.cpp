#include "bitboard.h"
#include "attacks.h"
#include "position.h"

#include <iostream>


namespace ChessEngine
{
	void init_all()
	{
		// Initialize attacks
		Attacks::init();

		// Initialize bitboards
		Bitboards::init();

		Color c = WHITE;

		// load the starting fen
		Position::init(ChessEngine::TEST_ATTACKS_FEN, c);
	}
}

	int main()
	{
		ChessEngine::init_all();

		system("pause");
		return 0;
	}