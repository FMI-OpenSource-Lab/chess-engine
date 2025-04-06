#pragma once

#include <cassert>
#include <iostream>
#include <array>
#include <vector>
#include <cstdint>

#include "defs.h"
#include "consts.h"

namespace KhaosChess
{
	constexpr auto BISHOP_ATTACKS_SIZE = 512;
	constexpr auto ROOK_ATTACKS_SIZE = 4096;

	// Declare prototypes
	constexpr inline BITBOARD set_occupancy(int index, int bitsInMask, BITBOARD attackMask);

	// extern void init_sliders_attacks(PieceType py);
	extern void init_pseudo_attacks();

	extern BITBOARD bishopAttacks(BITBOARD occ, Square s);
	extern BITBOARD rookAttacks(BITBOARD occ, Square s);

	constexpr BITBOARD square_to_BB(Square square)
	{
		assert(is_square_ok(square));
		return (1ULL << square);
	}

	namespace Bitboards
	{
		void init();
	};

	// overloads of bitwise operators between bitboard and square for testing purposes
	inline BITBOARD operator&(BITBOARD b, Square s) { return b & square_to_BB(s); }
	inline BITBOARD operator|(BITBOARD b, Square s) { return b | square_to_BB(s); }
	inline BITBOARD operator^(BITBOARD b, Square s) { return b ^ square_to_BB(s); }
	inline BITBOARD& operator|=(BITBOARD& b, Square s) { return b |= square_to_BB(s); }
	inline BITBOARD& operator^=(BITBOARD& b, Square s) { return b ^= square_to_BB(s); }

	inline BITBOARD operator&(Square s, BITBOARD b) { return b & s; }
	inline BITBOARD operator|(Square s, BITBOARD b) { return b | s; }
	inline BITBOARD operator^(Square s, BITBOARD b) { return b ^ s; }

	inline BITBOARD operator|(Square s1, Square s2) { return square_to_BB(s1) | s2; }

	// checks for available bit
	inline BITBOARD get_bit(BITBOARD bitboard, Square square) {
		return bitboard & square;
	}

	// set piece to square
	inline void set_bit(BITBOARD& bitboard, Square square) {
		bitboard |= square;
	}

	// if theres a 1 remove it, if 0 don't
	inline void rm_bit(BITBOARD& bitboard, Square square) {
		bitboard &= ~square_to_BB(square);
	}

	// Files
	constexpr BITBOARD FileA_Bits = 0x0101010101010101ULL; // first row is ones
	constexpr BITBOARD FileB_Bits = FileA_Bits << 1;
	constexpr BITBOARD FileC_Bits = FileA_Bits << 2;
	constexpr BITBOARD FileD_Bits = FileA_Bits << 3;
	constexpr BITBOARD FileE_Bits = FileA_Bits << 4;
	constexpr BITBOARD FileF_Bits = FileA_Bits << 5;
	constexpr BITBOARD FileG_Bits = FileA_Bits << 6;
	constexpr BITBOARD FileH_Bits = 0x8080808080808080ULL;

	// Ranks
	constexpr BITBOARD Rank8_Bits = 0xFF;
	constexpr BITBOARD Rank7_Bits = Rank8_Bits << (8 * 1);
	constexpr BITBOARD Rank6_Bits = Rank8_Bits << (8 * 2);
	constexpr BITBOARD Rank5_Bits = Rank8_Bits << (8 * 3);
	constexpr BITBOARD Rank4_Bits = Rank8_Bits << (8 * 4);
	constexpr BITBOARD Rank3_Bits = Rank8_Bits << (8 * 5);
	constexpr BITBOARD Rank2_Bits = Rank8_Bits << (8 * 6);
	constexpr BITBOARD Rank1_Bits = Rank8_Bits << (8 * 7);

	// Diagonals
	constexpr BITBOARD Diagonal_H1_A8 = 0x8040201008040201ULL;
	constexpr BITBOARD Diagonal_A1_H8 = 0x0102040810204080ULL;

	constexpr BITBOARD LightSquares = 0x55AA55AA55AA55AAULL;
	constexpr BITBOARD DarkSquares = 0xAA55AA55AA55AA55ULL;

	constexpr BITBOARD rank_bb(Square s) { return Rank8_Bits << (rank_of(s) << 3); }
	constexpr BITBOARD rank_bb(Rank r) { return Rank8_Bits << (8 * r); }
	constexpr BITBOARD file_bb(Square s) { return FileA_Bits << file_of(s); }
	constexpr BITBOARD file_bb(File f) { return FileA_Bits << f; }

	constexpr BITBOARD board_edges(Square s) { return ((Rank8_Bits | Rank1_Bits) & ~rank_bb(s)) | ((FileA_Bits | FileH_Bits) & ~file_bb(s)); }

	// Not files
	constexpr BITBOARD not_A = 18374403900871474942ULL;	// ~FileA_Bits bitboard value where the A file is set to zero
	constexpr BITBOARD not_H = 9187201950435737471ULL;	// ~FileH_Bits bitboard value where the H file is set to zero
	constexpr BITBOARD not_HG = 4557430888798830399ULL;	// ~FileH_Bits & ~FileG_Bits bitboard value where the HG files are set to zero
	constexpr BITBOARD not_AB = 18229723555195321596ULL;	// ~FileA_Bits & ~FileB_Bits bitboard value where the HG files are set to zero

	//// define magic bishop attack table [squares][occupancy]
	extern BITBOARD mBishopAttacks[SQUARE_TOTAL][BISHOP_ATTACKS_SIZE];
	//// define magic rook attack table [squares][occupancy]
	extern BITBOARD mRookAttacks[SQUARE_TOTAL][ROOK_ATTACKS_SIZE];

	// every pseudo attack for the given piece on the given square
	extern BITBOARD pseudo_attacks[PIECE_TYPE_NB][SQUARE_TOTAL];

	// pawn attacks table [side][square]
	extern BITBOARD pawn_attacks[2][64]; // 2 - sides to play, 64 - squares on a table

	// distance in squares between square x and square y
	extern unsigned short square_distance[SQUARE_TOTAL][SQUARE_TOTAL];

	// extern U64 bishop_attacks[];
	// extern U64 rook_attacks[];

	struct SMagic {
		BITBOARD* attack;
		BITBOARD mask;  // to mask relevant squares of both lines (no outer squares)
		BITBOARD magic; // magic 64-bit factor
		int shift; // shift relevant bits
	};

	extern SMagic bishop_magic_tbl[SQUARE_TOTAL];
	extern SMagic rook_magic_tbl[SQUARE_TOTAL];

	extern void init_magics(PieceType pt, SMagic magics[]);
	extern BITBOARD sliding_attacks(PieceType pt, Square s, BITBOARD occ);

	// moves the bb one or two steps
	template<Direction d>
	constexpr BITBOARD move_to(BITBOARD b)
	{
		return d == DOWN ? b << 8
			: d == UP ? b >> 8
			: d == DOWN + DOWN ? b << 16
			: d == UP + UP ? b >> 16
			: d == LEFT ? (b & not_A) >> 1
			: d == RIGHT ? (b & not_H) << 1
			: d == DOWN_LEFT ? (b & not_A) << 7
			: d == DOWN_RIGHT ? (b & not_H) << 9
			: d == UP_LEFT ? (b & not_A) >> 9
			: d == UP_RIGHT ? (b & not_H) >> 7
			: 0;
	}

	template<Color c>
	constexpr BITBOARD pawn_attacks_bb(BITBOARD bb)
	{
		return c == WHITE
			? move_to<UP_RIGHT>(bb) | move_to<UP_LEFT>(bb)
			: move_to<DOWN_RIGHT>(bb) | move_to<DOWN_LEFT>(bb);
	}

	constexpr BITBOARD pawn_attacks_bb(Color c, BITBOARD bb)
	{
		return c == WHITE
			? pawn_attacks_bb<WHITE>(bb)
			: pawn_attacks_bb<BLACK>(bb);
	}

	constexpr BITBOARD pawn_attacks_bb(Color c, Square bb)
	{
		return c == WHITE
			? pawn_attacks_bb<WHITE>(square_to_BB(bb))
			: pawn_attacks_bb<BLACK>(square_to_BB(bb));
	}

	template<PieceType pt>
	inline BITBOARD attacks_bb_by(Square s, BITBOARD occ)
	{
		assert(is_square_ok(s) && pt != PAWN);

		switch (pt)
		{
		case BISHOP:
			return bishopAttacks(occ, s);
		case ROOK:
			return rookAttacks(occ, s);
		case QUEEN:
			return bishopAttacks(occ, s) | rookAttacks(occ, s);
		default: // king and knight
			return pseudo_attacks[pt][s];
		}
	}

	inline BITBOARD attacks_bb_by(PieceType pt, Square s, BITBOARD occ)
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
	inline BITBOARD attacks_bb_by(Square s)
	{
		assert(is_square_ok(s) && (pt != PAWN));

		return pseudo_attacks[pt][s];
	}

	/*
		Base-2 integer logarithm

		Calculates the floor of the base-2 logarithm
		which is equivalent to the index of the most significant bit
	*/
	constexpr int u64_log2(BITBOARD n)
	{
		// Function written in set notation:
		// {2^(2^k) | 0 <= k <= 5}
		// eg: {2^32, 2^16, 2^8, 2^4, 2^2, 2^1}
		// And sums the exponents k of the subtracted values.
#define S(k) if (n >= (uint64_t(1) << k)) { i += k; n >>= k; }

		// returns -1 if input is 0, hence the bitboard is empty
		int i = -(n == 0); 
		S(32); S(16); S(8); S(4); S(2); S(1); return i;

		// does log2 faster than cmaths log2 function
#undef S
	}

	constexpr Square get_ls1b(BITBOARD bitboard)
	{
		// count trailing bits before LS1B
		int lsb = u64_log2(bitboard & -bitboard);

		return lsb == -1 ? NONE : Square(lsb);
	}

	// calculate the line that is formed between two points
	// return that line bitboard + the target square
	// or only the target square in case of knight attack
	inline BITBOARD in_between_bb(Square source, Square target)
	{
		// source and target have commutative properties
		// we can do [pt][source] & target or [pt][target] & source

		// check if pseudo legal move exists in this diagonal
		// and generate an attack thats going to serve as 
		// an inbetween squares excluding source and target
		if (pseudo_attacks[BISHOP][source] & target) return (
			attacks_bb_by(BISHOP, source, square_to_BB(target)) &
			attacks_bb_by(BISHOP, target, square_to_BB(source))) | target;

		// check if pseudo legal move exists in this line
		// generate an attack thats going to serve as an inbetween squares
		// excluding source and target
		else if (pseudo_attacks[ROOK][source] & target) return (
			attacks_bb_by(ROOK, source, square_to_BB(target)) &
			attacks_bb_by(ROOK, target, square_to_BB(source))) | target;

		// no inbetween squares will return the target square in case of knight attacks
		return square_to_BB(target);
	}

	// Returns true if all 3 squares are aligned
	inline bool are_squares_aligned(Square source, Square target, Square aligned_with)
	{
		BITBOARD bb{};

		for (auto& pt : { ROOK, BISHOP })
			if (pseudo_attacks[pt][source] & target)
				bb |= (attacks_bb_by(pt, source, 0) & (attacks_bb_by(pt, target, 0)));

		return (bb | source | target) & aligned_with;
	}

	inline void print_bitboard(BITBOARD bitboard)
	{
		std::cout << "\nBitboard representation:\n\n";

		for (Rank rank = RANK_8; rank <= RANK_1; ++rank)
		{
			for (File file = FILE_A; file <= FILE_H; ++file)
			{
				Square sq = make_square(file, rank);
				std::string s = get_bit(bitboard, sq) ? " 1" : " 0";

				std::cout << s;
			}

			std::cout << "  " << (8 - rank) << " \n";
		}

		// print files
		std::cout << "\n a b c d e f g h \n\n";

		// print as udec value
		std::cout << "\n Bitboard: " << bitboard << "\n\n";
	}

	constexpr inline BITBOARD set_occupancy(int index, int bitsInMask, BITBOARD attackMask)
	{
		// occupancy map
		BITBOARD occupancy = 0ULL;

		// loop on bits within attack mask
		for (int count = 0; count < bitsInMask; count++)
		{
			// get the lsb square
			Square square = get_ls1b(attackMask);

			// remove LSB
			rm_bit(attackMask, square);

			// make occupancy on board
			if (index & (1 << count))
				// populate occupancy map
				occupancy |= square_to_BB(square);
		}

		return occupancy;
	}

	constexpr bool has_bit_after_pop(BITBOARD bitboard)
	{
		return bitboard & (bitboard - 1);
	}

	constexpr Square pop_ls1b(BITBOARD& bitboard)
	{
		assert(bitboard);

		Square s = get_ls1b(bitboard);
		bitboard &= bitboard - 1;

		return s;
	}

	const uint64_t m1 = 0x5555555555555555;  //binary: 0101...
	const uint64_t m2 = 0x3333333333333333;  //binary: 00110011..
	const uint64_t m4 = 0x0f0f0f0f0f0f0f0f;  //binary:  4 zeros,  4 ones ...
	const uint64_t h01 = 0x0101010101010101; //the sum of 256 to the power of 0,1,2,3...

	// Hamming weight algorithm for finding the count of the 1's 
	constexpr int count_bits(BITBOARD bitboard)
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
