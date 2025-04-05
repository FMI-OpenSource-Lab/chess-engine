#pragma once
#ifndef RANDOM_H
#define RANDOM_H

#include <cstdint>
#include <vector>

namespace ChessEngine
{
	// pseudo random number generator
	class PRNG
	{
		uint64_t seed;

		// inspired by Stockfish
		uint64_t rand64() 
		{
			seed ^= seed >> 12, seed ^= seed << 25, seed ^= seed >> 27;
			return seed * 2685821657736338717LL;
		};

	public:
		PRNG(uint16_t seed) : seed(seed) {}

		template<typename T>

		T rand() { return T(rand64() & rand64() & rand64()); }
	};
}
#endif // !RANDOM_H
