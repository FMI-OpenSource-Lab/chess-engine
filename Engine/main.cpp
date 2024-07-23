#include "bitboard.h"
#include "attacks.h"

#include <iostream>

void init_all()
{
	initAttacks();

    init_sliders_attacks(BISHOP);
    init_sliders_attacks(ROOK);
}

int main()
{
	init_all();

	U64 occ = 0ULL;

	set_bit(occ, D6);

	print_bitboard(occ);

	print_bitboard(bishopAttacks(occ, F4));
	print_bitboard(rookAttacks(occ, D4));

	system("pause");
	return 0;
}