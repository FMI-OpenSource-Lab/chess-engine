#define NOMINMAX

#include<iostream>

#include "bitboard.h"
#include "defs.h"

namespace ChessEngine {
	unsigned short square_distance[SQUARE_TOTAL][SQUARE_TOTAL];

	U64 mBishopAttacks[SQUARE_TOTAL][512];
	U64 mRookAttacks[SQUARE_TOTAL][4096];

	U64 point_to_point_in_line_bb[SQUARE_TOTAL][SQUARE_TOTAL];
	U64 between_points_bb[SQUARE_TOTAL][SQUARE_TOTAL];
	U64 pseudo_attacks[PIECE_TYPE_NB][SQUARE_TOTAL];

	SMagic mBishopTbl[SQUARE_TOTAL];
	SMagic mRookTbl[SQUARE_TOTAL];

	void init_sliders_attacks(PieceType py)
	{
		// loop over 64 board squares
		for (Square sq = A8; sq <= H1; ++sq)
		{
			// init bishop & rook masks
			mBishopTbl[sq].mask = bishop_attacks_mask(sq);
			mRookTbl[sq].mask = rook_attacks_mask(sq);

			// init current mask
			U64 attack_mask =
				py == BISHOP
				? mBishopTbl[sq].mask
				: mRookTbl[sq].mask;

			// init relevant occupancy bit count
			int relevant_bits_count = countBits(attack_mask);

			// init occupancy indicies
			int occupancy_indicies = (1 << relevant_bits_count);

			// init magic index
			size_t magic_index = 0;

			// loop over occupancy indicies
			for (int index = 0; index < occupancy_indicies; index++)
			{
				// bishop
				if (py == BISHOP)
				{
					// init current occupancy variation
					U64 occupancy = set_occupancy(index, relevant_bits_count, attack_mask);

					// init magic index
					magic_index = (occupancy * BISHOP_MAGIC_NUMBERS[square]) >> (64 - RELEVANT_BISHOP_BITS[square]);
					mBishopTbl[square].magic = BISHOP_MAGIC_NUMBERS[square];

					// init bishop attacks
					mBishopAttacks[square][magic_index] = bishop_attacks_generate(sq, occupancy);
				}

				// rook
				else if (py == ROOK)
				{
					// init current occupancy variation
					U64 occupancy = set_occupancy(index, relevant_bits_count, attack_mask);

					// init magic index
					magic_index = (occupancy * ROOK_MAGIC_NUMBERS[square]) >> (64 - RELEVANT_ROOK_BITS[square]);
					mRookTbl[square].magic = ROOK_MAGIC_NUMBERS[square];

					// init bishop attacks
					mRookAttacks[square][magic_index] = rook_attacks_generate(sq, occupancy);
				}
			}
		}
	}

	U64 siding_attacks(PieceType pt, Square s, U64 occ)
	{
		U64 attacks = 0ULL;
		Direction rook_dirs[4] = { NORTH, SOUTH, EAST, WEST };
		Direction bishop_dirs[4] = { NORTH_EAST, SOUTH_EAST, SOUTH_WEST, NORTH_WEST };

		for (Direction d : (pt == ROOK ? rook_dirs : bishop_dirs))
		{
			Square source = s;
			Square target = Square(source + d);

			U64 distance_bb = square_distance[source][target] <= 2 ? square_to_BB(target) : U64(0);

			while (is_square_ok(target) && distance_bb)
			{
				attacks |= (source += d);

				if (occ & source)
					break;
			}
		}

		return attacks;
	}

	U64 bishopAttacks(U64 occ, Square sq)
	{
		occ &= mBishopTbl[sq].mask;
		occ *= mBishopTbl[sq].magic;
		occ >>= 64 - RELEVANT_BISHOP_BITS[sq];

		return mBishopAttacks[sq][occ];
	}

	U64 rookAttacks(U64 occ, Square sq)
	{
		occ &= mRookTbl[sq].mask;
		occ *= mRookTbl[sq].magic;
		occ >>= 64 - RELEVANT_ROOK_BITS[sq];

		return mRookAttacks[sq][occ];
	}

	U64 queenAttacks(U64 occ, Square sq)
	{
		return 0ULL;
	}

	void init_pseudo_attacks()
	{
		int king_offsets[8] = { -9, -8, -7, -1, 1, 7, 8, 9 };
		int knight_offsets[8] = { -17, -15, -10, -6, 6, 10, 15, 17 };

		for (Square s = A8; s <= H1; ++s)
		{
			// generate the pawn attacks
			pawnAttacks[WHITE][s] = pawn_attacks_bb<WHITE>(square_to_BB(s));
			pawnAttacks[BLACK][s] = pawn_attacks_bb<BLACK>(square_to_BB(s));

			// generate pseudo attacks for king
			pseudo_attacks[KNIGHT][s] |= king_attacks_mask(s);

			// generate pseudo attacks for knight
			pseudo_attacks[KNIGHT][s] |= knight_attacks_mask(s);

			// generate pseudo attacks for rook and bishop
			pseudo_attacks[BISHOP][s] = attacks_bb<BISHOP>(s, 0ULL);
			pseudo_attacks[ROOK][s] = attacks_bb<ROOK>(s, 0ULL);

			// generate pseudo attacks for queen
			pseudo_attacks[QUEEN][s] =
				pseudo_attacks[BISHOP][s] | pseudo_attacks[ROOK][s];

			// calculate the line that is formed between two points
			// bishops because they are the only pieces that can move horizontaly and diagonaly (except the queen, but she is a combination of rook and bishop)
			for (Square pa_square = A8; pa_square <= H1; ++pa_square)
			{
				if (pseudo_attacks[BISHOP][s] & pa_square)
				{
					point_to_point_in_line_bb[s][pa_square] =
						(attacks_bb<BISHOP>(s, 0ULL) & attacks_bb<BISHOP>(pa_square, 0ULL)) | s | pa_square;

					between_points_bb[s][pa_square] =
						(attacks_bb<BISHOP>(s, square_to_BB(pa_square)) & attacks_bb<BISHOP>(pa_square, square_to_BB(s)));
				}

				if (pseudo_attacks[ROOK][s] & pa_square)
				{
					point_to_point_in_line_bb[s][pa_square] = (attacks_bb<ROOK>(s, 0ULL) & attacks_bb<ROOK>(pa_square, 0ULL)) | s | pa_square;

					between_points_bb[s][pa_square] =
						(attacks_bb<ROOK>(s, square_to_BB(pa_square)) & attacks_bb<ROOK>(pa_square, square_to_BB(s)));
				}

				between_points_bb[s][pa_square] |= pa_square;
			}
		}
	}

	void Bitboards::init()
	{
		init_sliders_attacks(BISHOP);
		init_sliders_attacks(ROOK);

		for (Square x = A8; x <= H8; ++x)
			for (Square y = A8; y <= H8; ++y)
				square_distance[x][y] = std::max(
					std::abs(file_of(x) - file_of(y)),
					std::abs(rank_of(x) - rank_of(y)));

		init_pseudo_attacks();
	}
}