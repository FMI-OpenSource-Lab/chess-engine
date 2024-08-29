#pragma once
#ifndef BITBOARD_H
#define BITBOARD_H

#include <cassert>
#include <cstdint>
#include <iostream>
#include <array>

#include "consts.h"

namespace ChessEngine
{
	// Declare prototypes
	constexpr inline U64 set_occupancy(int index, int bitsInMask, U64 attackMask);

	extern constexpr Square getLS1B_square(U64 bitboard);

	extern constexpr int getLS1B(U64 bitboard);
	extern constexpr void resetLSB(U64& bitboard);
	extern constexpr int countBits(U64 bitboard);

	void init_sliders_attacks(PieceType py);
	extern U64 bishopAttacks(U64 occ, Square sq);
	extern U64 rookAttacks(U64 occ, Square sq);
	extern U64 queenAttacks(U64 occ, Square sq);

	constexpr U64 squareBB(Square square)
	{
		return (1ULL << square);
	}

	namespace Bitboards
	{
		void init();
	};

	// overloads of bitwise operators between bitboard and square for testing purposes
	inline U64 operator&(U64 b, Square s) { return b & squareBB(s); }
	inline U64 operator|(U64 b, Square s) { return b | squareBB(s); }
	inline U64 operator^(U64 b, Square s) { return b ^ squareBB(s); }
	inline U64& operator|=(U64 b, Square s) { return b |= squareBB(s); }
	inline U64& operator^=(U64 b, Square s) { return b ^= squareBB(s); }

	inline U64 operator&(Square s, U64 b) { return b & s; }
	inline U64 operator|(Square s, U64 b) { return b | s; }
	inline U64 operator^(Square s, U64 b) { return b ^ s; }

	inline U64 operator|(Square s1, Square s2) { return squareBB(s1) | s2; }

	// Files
	constexpr U64 FileA_Bits = 0x0101010101010101ULL; // first row is ones
	constexpr U64 FileB_Bits = FileA_Bits << 1;
	constexpr U64 FileC_Bits = FileA_Bits << 2;
	constexpr U64 FileD_Bits = FileA_Bits << 3;
	constexpr U64 FileE_Bits = FileA_Bits << 4;
	constexpr U64 FileF_Bits = FileA_Bits << 5;
	constexpr U64 FileG_Bits = FileA_Bits << 6;
	constexpr U64 FileH_Bits = 0x8080808080808080ULL;

	// Ranks
	constexpr U64 Rank8_Bits = 0xFF;
	constexpr U64 Rank7_Bits = Rank8_Bits << (8 * 1);
	constexpr U64 Rank6_Bits = Rank8_Bits << (8 * 2);
	constexpr U64 Rank5_Bits = Rank8_Bits << (8 * 3);
	constexpr U64 Rank4_Bits = Rank8_Bits << (8 * 4);
	constexpr U64 Rank3_Bits = Rank8_Bits << (8 * 5);
	constexpr U64 Rank2_Bits = Rank8_Bits << (8 * 6);
	constexpr U64 Rank1_Bits = Rank8_Bits << (8 * 7);

	// Diagonals
	constexpr U64 Diagonal_H1_A8 = 0x8040201008040201ULL;
	constexpr U64 Diagonal_A1_H8 = 0x0102040810204080ULL;

	constexpr U64 LightSquares = 0x55AA55AA55AA55AAULL;
	constexpr U64 DarkSquares = 0xAA55AA55AA55AA55ULL;

	// Not files
	constexpr U64 not_A = 18374403900871474942ULL;	// ~FileA_Bits bitboard value where the A file is set to zero
	constexpr U64 not_H = 9187201950435737471ULL;	// ~FileH_Bits bitboard value where the H file is set to zero
	constexpr U64 not_HG = 4557430888798830399ULL;	// ~FileH_Bits & ~FileG_Bits bitboard value where the HG files are set to zero
	constexpr U64 not_AB = 18229723555195321596ULL;	// ~FileA_Bits & ~FileB_Bits bitboard value where the HG files are set to zero

	// define magic bishop attack table [squares][occupancy]
	extern U64 mBishopAttacks[64][512]; // 256 K

	// define magic rook attack table [squares][occupancy]
	extern U64 mRookAttacks[64][4096]; // 2048K

	struct SMagic {
		U64 mask;  // to mask relevant squares of both lines (no outer squares)
		U64 magic; // magic 64-bit factor
	};

	inline void print_bitboard(U64 bitboard)
	{
		std::cout << std::endl;

		// loop on ranks
		for (unsigned short rank = 0; rank < 8; rank++)
		{
			// loop on files
			for (unsigned short file = 0; file < 8; file++)
			{
				// convert rank and file into square index
				int sq = rank * 8 + file;

				if (!file) printf("  %d ", 8 - rank);

				// print bit state (1 or 0)
				printf(" %d", get_bit(bitboard, sq) ? 1 : 0);
			}

			std::cout << std::endl;
		}

		// print files
		std::cout << "\n     a b c d e f g h \n\n";

		// print as udec value
		std::cout << "     Bitboard: " << bitboard << "\n\n";
	}

	constexpr inline U64 set_occupancy(int index, int bitsInMask, U64 attackMask)
	{
		// occupancy map
		U64 occupancy = 0ULL;

		// loop on bits within attack mask
		for (int count = 0; count < bitsInMask; count++)
		{
			// get the lsb square
			int square = getLS1B(attackMask);

			// remove LSB
			rm_bit(attackMask, square);

			// make occupancy on board
			if (index & (1 << count))
				// populate occupancy map
				occupancy |= (1ULL << square);
		}

		return occupancy;
	}

	/* 
		Base-2 integer logarithm

		Calculates the floor of the base-2 logarithm
		which is equivalent to the index of the most significant bit
	*/
	constexpr int u64_log2(U64 n)
	{
		// Function written in set notation:
		// {2^(2^k) | 0 <= k <= 5}
		// eg: {2^32, 2^16, 2^8, 2^4, 2^2, 2^1}
		// And sums the exponents k of the subtracted values.
#define S(k) if (n >= (UINT64_C(1) << k)) { i += k; n >>= k; }

		// returns -1 if input is 0, hence the bitboard is empty
		int i = -(n == 0); S(32); S(16); S(8); S(4); S(2); S(1); return i;

		// does log2 faster than cmaths log2 function
#undef S
	}

	constexpr int getLS1B(U64 bitboard)
	{
		// check if bb is not 0
		if (bitboard)
		{
			// count trailing bits before LS1B
			U64 twosComplement = ~bitboard + 1; // -bitboard is equivelent
			return std::log2(bitboard & twosComplement);
		}
		else
			return -1;
	}

	constexpr Square getLS1B_square(U64 bitboard)
	{
		int lsb = getLS1B(bitboard);
		return lsb == -1 ? NONE : get_square(lsb);
	}

	constexpr void resetLSB(U64& bitboard)
	{
		bitboard &= bitboard - 1;
	}

	// Counting algorithm that works better where most bits in bitboard are 0
	constexpr int countBits(U64 bitboard)
	{
		int count = 0;

		for (count = 0; bitboard; count++)
			resetLSB(bitboard);

		return count;
	}

	const uint64_t m1 = 0x5555555555555555; //binary: 0101...
	const uint64_t m2 = 0x3333333333333333; //binary: 00110011..
	const uint64_t m4 = 0x0f0f0f0f0f0f0f0f; //binary:  4 zeros,  4 ones ...
	const uint64_t h01 = 0x0101010101010101; //the sum of 256 to the power of 0,1,2,3...

	// Hamming weight algorithm for finding the count of the 1's 
	constexpr int count_bits_hamming_weight(U64 bitboard)
	{
		bitboard -= (bitboard >> 1) & m1;             //put count of each 2 bits into those 2 bits
		bitboard = (bitboard & m2) + ((bitboard >> 2) & m2); //put count of each 4 bits into those 4 bits 
		bitboard = (bitboard + (bitboard >> 4)) & m4;        //put count of each 8 bits into those 8 bits 

		return (bitboard * h01) >> 56;  //returns left 8 bits of x + (x<<8) + (x<<16) + (x<<24) + ... 
	}
}
#endif // !BITBOARD_H
