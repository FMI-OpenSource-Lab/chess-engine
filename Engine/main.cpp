#include "bitboard.h"
#include "attacks.h"
#include "position.h"
#include "movegen.h"
#include "perft.h"
#include "move.h"
#include "uci.h"
#include "score.h"
#include "search.h"

#include <ostream>
#include <bitset>

using namespace ChessEngine;

void init_all()
{
	// Initialize attacks
	Attacks::init();

	// Initialize bitboards
	Bitboards::init();
}

int main()
{
	init_all();
	bool debug = true;

	if (debug)
	{
		std::cout << "Debugging\n\n";

		Position::init(START_FEN);
		print_board();

		Move a{ B7, C8};
		a = a.make<MT_PROMOTION>(a.source_square(), a.target_square(), QUEEN);

		std::string move_value = std::bitset<24>(a.move_value()).to_string();

		std::cout << "\nraw:	" << a.move_value() << "\n";
		std::cout << "raw binary:\n";

		int iter = 0;
		for (size_t i = 0; i < move_value.length(); i++)
		{
			if (i % 5 == 0)
			{
				move_value.insert(i, " ");
				// iter++;
			}
		}
		move_value[0] = '\0';

		std::cout << move_value << "\n";

		std::cout << "\npromoted: " << a.promoted() << "\n";
		std::cout << "move type: " << a.move_type() << "\n";

		std::cout << a;

		//search_position(2);
	}
	else
		uci_loop();

	system("pause");
	return 0;
}