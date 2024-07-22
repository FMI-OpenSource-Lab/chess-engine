#include "attacks.h"
#include "defs.h"
#include "random.h"

#include <iostream>

int main()
{
	initAttacks();

	//Attacks attacks;
	////Bitboard bb;
	PRNG prng;

	//print_bitboard((U64)prng.rand32());
	//print_bitboard((U64)(prng.rand32() & 0xFFFF));
	//print_bitboard(prng.rand64());
	//print_bitboard(prng.rand64() & prng.rand64() & prng.rand64());

	//print_bitboard(Rank8_Bits);

	prng.init_magic_numbers();

	system("pause");
	return 0;
}