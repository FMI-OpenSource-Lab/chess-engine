#pragma once
#ifndef RANDOM_H
#define RANDOM_H

#include "attacks.h"

// pseudo random number generator
class PRNG
{
public:
	uint32_t random_state = 1804289383;

	// generate 32-bit pseudo legal nums
	uint32_t rand32()
	{
		// get curr state
		unsigned int currentState = random_state;

		// XOR shift algorithm (XORSHIFT32)
		currentState ^= currentState << 13;
		currentState ^= currentState >> 17;
		currentState ^= currentState << 5;

		// update random number state
		random_state = currentState;

		// ret random number
		return currentState;
	}

	// generate 64-bit pseudo legal numbers
	U64 rand64()
	{
		// define 4 random numbers
		U64 n1, n2, n3, n4;

		// init random numbers slicing 16 bits from MS1b
		n1 = (U64)(rand32()) & 0xFFFF;
		n2 = (U64)(rand32()) & 0xFFFF;
		n3 = (U64)(rand32()) & 0xFFFF;
		n4 = (U64)(rand32()) & 0xFFFF;

		// return random number
		return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
	}

	// magic number candidate
	U64 magic_number()
	{
		U64 n = rand64();
		return n & n & n;
	}

	// find magic number
	U64 find_magic(Square square, int relevant_bits, int bishop)
	{
		// init occupancies
		U64 occupancies[4096];

		// init attack tables
		U64 attacks[4096];

		// init used attacks
		U64 used_attacks[4096];

		// init attack mask for current piece
		U64 attack_mask = bishop ? maskBishopAttacks(square) : maskRookAttacks(square);

		// init occupancie indecies
		int occupancy_indicies = 1 << relevant_bits;

		// loop over occupancy indicies
		for (int index = 0; index < occupancy_indicies; index++)
		{
			// init occ
			occupancies[index] = set_occupancy(index, relevant_bits, attack_mask);

			// init attacks
			attacks[index] = bishop ? generateBishopAttacks(square, occupancies[index]) :
				generateRookAttacks(square, occupancies[index]);
		}

		for (int random_count = 0; random_count < 100000000; random_count++)
		{
			// generate magic number candidate
			U64 magic_number = rand64();

			if (countBits((attack_mask * magic_number) & 0xFF) < 6) continue;

			memset(used_attacks, 0ULL, sizeof(used_attacks));

			// flags
			int index, fail;

			// test magic index
			for (index = 0, fail = 0; !fail && index < occupancy_indicies; index++)
			{
				// init magic index
				int magic_index = (int)((occupancies[index] * magic_number) >> (64 - relevant_bits));

				// if magic index works
				if (used_attacks[index] == 0ULL)
					used_attacks[index] = attacks[index]; // init attacks
				else if (used_attacks[index] != attacks[index])
					fail = 1;
			}

			if (!fail)
				return magic_number;
		}

		printf("Magic number fails!");
		return 0ULL;
	}

	//PRNG(uint32_t seed) : state(seed) {}
};

#endif // !RANDOM_H
