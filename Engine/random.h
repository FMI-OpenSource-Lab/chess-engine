#pragma once
#ifndef RANDOM_H
#define RANDOM_H

#include "attacks.h"

// pseudo random number generator
class PRNG
{
public:
	uint32_t state = 1804289383;

	// generate 32-bit pseudo legal nums
	uint32_t rand32()
	{
		// get curr state
		unsigned int currentState = state;

		// XOR shift algorithm (XORSHIFT32)
		currentState ^= currentState << 13;
		currentState ^= currentState >> 17;
		currentState ^= currentState << 5;

		// update random number state
		state = currentState;

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
	U64 find_magic(Square square, int relevant_bits, U64 attack_mask, int bishop)
	{
		// init occupancies
		U64 occupancies[4096];

		// init attack tables
		U64 attacks[4096];

		// init used attacks
		U64 used_attacks[4096];

		// TODO: DO NOT DO THIS, NOT EFFICIENT 
		// i did it only for testing purposes
		Attacks att;

		// init attack mask for current piece
		U64 attack_mask = bishop ? att.maskBishopAttacks(square) : att.maskRookAttacks(square);

		// init occupancie indecies
		int occupancy_indicies = 1 << relevant_bits;

		// loop over occupancy indicies
		for (int index = 0; index < occupancy_indicies; index++)
		{
			// init occ
			occupancies[index] = set_occupancy(index, relevant_bits, attack_mask);

			// init attacks
			attacks[index] = bishop ? att.generateBishopAttacks(square, occupancies[index]) :
				att.generateRookAttacks(square, occupancies[index]);
		}

		// To be continued..
	}

	//PRNG(uint32_t seed) : state(seed) {}
};

#endif // !RANDOM_H
