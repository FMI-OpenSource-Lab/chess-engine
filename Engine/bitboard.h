#pragma once
#ifndef BITBOARD_H
#define BITBOARD_H

#include <cassert>
#include <iostream>
#include <array>
#include <vector>

#include "consts.h"

namespace ChessEngine
{
#define BISHOP_ATTACKS_SIZE 512
#define ROOK_ATTACKS_SIZE 4096

	// Declare prototypes
	constexpr inline U64 set_occupancy(int index, int bitsInMask, U64 attackMask);

	extern constexpr Square getLS1B_square(U64 bitboard);

	extern constexpr int getLS1B(U64 bitboard);
	extern constexpr void resetLSB(U64& bitboard);
	extern constexpr int countBits(U64 bitboard);

	// extern void init_sliders_attacks(PieceType py);
	extern void init_pseudo_attacks();

	extern U64 bishopAttacks(U64 occ, Square s);
	extern U64 rookAttacks(U64 occ, Square s);

	constexpr U64 square_to_BB(Square square)
	{
		return (1ULL << square);
	}

	namespace Bitboards
	{
		void init();
	};

	// overloads of bitwise operators between bitboard and square for testing purposes
	inline U64 operator&(U64 b, Square s) { return b & square_to_BB(s); }
	inline U64 operator|(U64 b, Square s) { return b | square_to_BB(s); }
	inline U64 operator^(U64 b, Square s) { return b ^ square_to_BB(s); }
	inline U64& operator|=(U64& b, Square s) { return b |= square_to_BB(s); }
	inline U64& operator^=(U64& b, Square s) { return b ^= square_to_BB(s); }

	inline U64 operator&(Square s, U64 b) { return b & s; }
	inline U64 operator|(Square s, U64 b) { return b | s; }
	inline U64 operator^(Square s, U64 b) { return b ^ s; }

	inline U64 operator|(Square s1, Square s2) { return square_to_BB(s1) | s2; }

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

	constexpr U64 rank_bb(Square s) { return Rank8_Bits << (rank_of(s) << 3); }
	constexpr U64 file_bb(Square s) { return FileA_Bits << file_of(s); }

	constexpr U64 board_edges(Square s) { return ((Rank8_Bits | Rank1_Bits & ~rank_bb(s)) | (FileA_Bits | FileH_Bits) & ~file_bb(s)); }

	// Not files
	constexpr U64 not_A = 18374403900871474942ULL;	// ~FileA_Bits bitboard value where the A file is set to zero
	constexpr U64 not_H = 9187201950435737471ULL;	// ~FileH_Bits bitboard value where the H file is set to zero
	constexpr U64 not_HG = 4557430888798830399ULL;	// ~FileH_Bits & ~FileG_Bits bitboard value where the HG files are set to zero
	constexpr U64 not_AB = 18229723555195321596ULL;	// ~FileA_Bits & ~FileB_Bits bitboard value where the HG files are set to zero

	//// define magic bishop attack table [squares][occupancy]
	extern U64 mBishopAttacks[SQUARE_TOTAL][BISHOP_ATTACKS_SIZE];
	//// define magic rook attack table [squares][occupancy]
	extern U64 mRookAttacks[SQUARE_TOTAL][ROOK_ATTACKS_SIZE];

	// every pseudo attack for the given piece on the given square
	extern U64 pseudo_attacks[PIECE_TYPE_NB][SQUARE_TOTAL];

	// pawn attacks table [side][square]
	extern U64 pawn_attacks[2][64]; // 2 - sides to play, 64 - squares on a table

	// distance in squares between square x and square y
	extern unsigned short square_distance[SQUARE_TOTAL][SQUARE_TOTAL];

	// extern U64 bishop_attacks[];
	// extern U64 rook_attacks[];

	struct SMagic {
		U64* attack;
		U64 mask;  // to mask relevant squares of both lines (no outer squares)
		U64 magic; // magic 64-bit factor
		int shift; // shift relevant bits
	};

	extern SMagic bishop_magic_tbl[SQUARE_TOTAL];
	extern SMagic rook_magic_tbl[SQUARE_TOTAL];

	extern void init_magics(PieceType pt, SMagic magics[]);
	extern U64 sliding_attacks(PieceType pt, Square s, U64 occ);

	// moves the bb one or two steps
	template<Direction d>
	constexpr U64 move_to(U64 b)
	{
		return d == DOWN ? b << 8
			: d == UP ? b >> 8
			: d == DOWN + DOWN ? b << 16
			: d == UP + UP ? b >> 16
			: d == LEFT ? (b & not_H) << 1
			: d == RIGHT ? (b & not_A) >> 1
			: d == DOWN_LEFT ? (b & not_H) << 9
			: d == DOWN_RIGHT ? (b & not_A) << 7
			: d == UP_LEFT ? (b & not_H) >> 7
			: d == UP_RIGHT ? (b & not_A) >> 9
			: 0;
	}

	template<Color c>
	constexpr U64 pawn_attacks_bb(U64 bb)
	{
		return c == WHITE
			? move_to<UP_RIGHT>(bb) | move_to<UP_LEFT>(bb)
			: move_to<DOWN_RIGHT>(bb) | move_to<DOWN_LEFT>(bb);
	}

	constexpr U64 pawn_attacks_bb(Color c, U64 bb)
	{
		return c == WHITE
			? pawn_attacks_bb<WHITE>(bb)
			: pawn_attacks_bb<BLACK>(bb);
	}

	template<PieceType pt>
	inline U64 attacks_bb_by(Square s, U64 occ)
	{
		assert(is_square_ok(s) && pt != PAWN);

		switch (pt)
		{
		case BISHOP:
			return bishopAttacks(occ, s);
		case ROOK:
			return rookAttacks(occ, s);
		case QUEEN:
			return attacks_bb_by<BISHOP>(s, occ) | attacks_bb_by<ROOK>(s, occ);
		default: // king and knight
			return pseudo_attacks[pt][s];
		}
	}

	inline U64 attacks_bb_by(PieceType pt, Square s, U64 occ)
	{
		switch (pt)
		{
		case BISHOP:
			return attacks_bb_by<BISHOP>(s, occ);
		case ROOK:
			return attacks_bb_by<ROOK>(s, occ);
		case QUEEN:
			return attacks_bb_by<QUEEN>(s, occ);
		default: // king and knight
			return pseudo_attacks[pt][s];
		}
	}

	// For easier acces to knight and king attacks
	template<PieceType pt>
	inline U64 attacks_bb_by(Square s)
	{
		assert(is_square_ok(s) && (pt != PAWN));

		return pseudo_attacks[pt][s];
	}

	// calculate the line that is formed between two points
	// for bishops and rooks because they are the only pieces
	// that can move horizontaly or diagonaly 
	// (except the queen, but she is a combination of rook and bishop)
	inline U64 in_between_bb(Square source, Square target)
	{
		PieceType pt = NO_PIECE_TYPE;

		// source and target have commutative properties
		// we can do [pt][source] & target or [pt][target] & source

		// check if pseudo legal move exists in this diagonal
		// and generate an attack thats going to serve as 
		// an inbetween squares excluding source and target
		if (pseudo_attacks[BISHOP][source] & target) return
			attacks_bb_by(BISHOP, source, square_to_BB(target)) &
			attacks_bb_by(BISHOP, target, square_to_BB(source));

		// check if pseudo legal move exists in this line
		// generate an attack thats going to serve as an inbetween squares
		// excluding source and target
		else if (pseudo_attacks[ROOK][source] & target) return
			attacks_bb_by(ROOK, source, square_to_BB(target)) &
			attacks_bb_by(ROOK, target, square_to_BB(source));

		// no inbetween squares
		return 0ULL;
	}

	inline U64 aligned_squares_bb(Square source, Square target, Square king_sq)
	{
		U64 diagonal = (
			(attacks_bb_by<BISHOP>(source, 0)) &
			(attacks_bb_by<BISHOP>(source, 0)) | source | target
			);

		// Get the rook attacks from the source square to the target square and intersect with the king square
		U64 horizontal = (
			(attacks_bb_by<ROOK>(source, 0)) &
			(attacks_bb_by<ROOK>(source, 0)) | source | target
			);

		return diagonal | horizontal | king_sq;
	}

	inline void print_bitboard(U64 bitboard)
	{
		std::cout << std::endl;

		for (Rank rank = RANK_1; rank <= RANK_8; ++rank)
		{
			for (File file = FILE_A; file <= FILE_H; ++file)
			{
				Square sq = make_square(file, rank);
				std::string s = get_bit(bitboard, sq) ? " 1" : " 0";

				if (file == FILE_A)
					std::cout << "  " << (8 - rank) << " ";

				std::cout << s;
			}

			std::cout << "\n";
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
			Square square = getLS1B_square(attackMask);

			// remove LSB
			rm_bit(attackMask, square);

			// make occupancy on board
			if (index & (1 << count))
				// populate occupancy map
				occupancy |= square_to_BB(square);
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
			return u64_log2(bitboard & twosComplement);
		}
		else
			return -1;
	}

	constexpr Square getLS1B_square(U64 bitboard)
	{
		int lsb = getLS1B(bitboard);
		return lsb == -1 ? NONE : Square(lsb);
	}

	constexpr void resetLSB(U64& bitboard)
	{
		bitboard &= bitboard - 1;
	}

	// Counting algorithm that works better where most bits in bitboard are 0
	constexpr int countBits(U64 bitboard)
	{
		int count = 0;
		for (count = 0; bitboard; count++, bitboard &= bitboard - 1);
		return count;
	}

	const uint64_t m1 = 0x5555555555555555;  //binary: 0101...
	const uint64_t m2 = 0x3333333333333333;  //binary: 00110011..
	const uint64_t m4 = 0x0f0f0f0f0f0f0f0f;  //binary:  4 zeros,  4 ones ...
	const uint64_t h01 = 0x0101010101010101; //the sum of 256 to the power of 0,1,2,3...

	// Hamming weight algorithm for finding the count of the 1's 
	constexpr int count_bits_hamming_weight(U64 bitboard)
	{
		//put count of each 2 bits into those 2 bits
		bitboard -= (bitboard >> 1) & m1;
		//put count of each 4 bits into those 4 bits 
		bitboard = (bitboard & m2) + ((bitboard >> 2) & m2);
		//put count of each 8 bits into those 8 bits 
		bitboard = (bitboard + (bitboard >> 4)) & m4;

		//returns left 8 bits of x + (x<<8) + (x<<16) + (x<<24) + ... 
		return (bitboard * h01) >> 56;
	}
}
#endif // !BITBOARD_H
