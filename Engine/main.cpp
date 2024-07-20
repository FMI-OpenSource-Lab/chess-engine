#include "bitboard.h"
#include "attacks.h"

#include <iostream>
#include "random.h"

int main()
{
	Attacks attacks;
	//Bitboard bb;
	PRNG prng;

	print_bitboard((U64)prng.rand32());
	print_bitboard((U64)(prng.rand32() & 0xFFFF));
	print_bitboard(prng.rand64());
	print_bitboard(prng.rand64() & prng.rand64() & prng.rand64());

	return 0;
}