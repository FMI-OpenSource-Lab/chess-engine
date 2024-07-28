#include "bitboard.h"
#include "attacks.h"
#include "position.h"

#include <iostream>

void print_attacked_squares(Color c)
{
	std::cout << std::endl;

	for (Rank rank = RANK_1; rank <= RANK_8; ++rank)
	{
		for (File file = FILE_A; file <= FILE_H; ++file)
		{
			Square s = get_square(rank, file);

			if (!file)
				printf(" %d ", 8 - rank);

			printf(" %d", is_attacked(s, c) ? 1: 0);
		}

		std::cout << std::endl;
	}

	printf("\n	 a b c d e f g h\n\n");
}

void init_all()
{
	// Initialize attacks
	initAttacks();

	// Initialize bitboards
	Bitboards::init();

	// load the starting fen
	Position::init(TEST_ATTACKS_FEN);

	print_board();
	print_attacked_squares(BLACK);
}

int main()
{
	init_all();

	system("pause");
	return 0;
}