#define NOMINMAX

#include "bitboard.h"
#include "defs.h"
#include "random.h"

namespace ChessEngine {
	// Table measuring the distance between 2 coordinates
	unsigned short square_distance[SQUARE_TOTAL][SQUARE_TOTAL];

	// a line that connects two coordinates
	U64 point_to_point_in_line_bb[SQUARE_TOTAL][SQUARE_TOTAL];

	// squares between 2 points
	U64 between_points_bb[SQUARE_TOTAL][SQUARE_TOTAL];

	// pseudo legal attacks
	U64 pseudo_attacks[PIECE_TYPE_NB][SQUARE_TOTAL];

	// pawn attacks, also pseudo legal
	U64 pawn_attacks[BOTH][SQUARE_TOTAL];

	// Magic rook and bishop tables
	SMagic bishop_magic_tbl[SQUARE_TOTAL];
	SMagic rook_magic_tbl[SQUARE_TOTAL];

	U64 mBishopAttacks[SQUARE_TOTAL][BISHOP_ATTACKS_SIZE]; // Bishop attacks
	U64 mRookAttacks[SQUARE_TOTAL][ROOK_ATTACKS_SIZE]; // Rook attacks

	namespace { 
		// Masks for king and knight squares 
		U64 knight_attacks_mask(const Square& square);
		U64 king_attacks_mask(const Square& square);
	}

	void Bitboards::init()
	{
		// Calculate the square distance
		for (Square x = A8; x <= H1; ++x)
			for (Square y = A8; y <= H1; ++y)
				square_distance[x][y] = std::max(
					std::abs(file_of(x) - file_of(y)),
					std::abs(rank_of(x) - rank_of(y)));

		// initiallize magic squares
		init_magics(ROOK, rook_magic_tbl);
		init_magics(BISHOP, bishop_magic_tbl);

		// initialize peseudo attacks
		init_pseudo_attacks();
	}

	void init_pseudo_attacks()
	{
		for (Square s = A8; s <= H1; ++s)
		{
			// generate the pawn attacks
			pawn_attacks[WHITE][s] = pawn_attacks_bb<WHITE>(square_to_BB(s));
			pawn_attacks[BLACK][s] = pawn_attacks_bb<BLACK>(square_to_BB(s));

			// generate pseudo attacks for king
			pseudo_attacks[KNIGHT][s] |= king_attacks_mask(s);

			// generate pseudo attacks for knight
			pseudo_attacks[KNIGHT][s] |= knight_attacks_mask(s);

			// generate pseudo attacks for rook and bishop
			pseudo_attacks[BISHOP][s] = attacks_bb_by<BISHOP>(s, 0ULL);
			pseudo_attacks[ROOK][s] = attacks_bb_by<ROOK>(s, 0ULL);

			// generate pseudo attacks for queen
			pseudo_attacks[QUEEN][s] =
				pseudo_attacks[BISHOP][s] | pseudo_attacks[ROOK][s];

			// calculate the line that is formed between two points
			// for bishops and rooks because they are the only pieces that can move horizontaly and diagonaly (except the queen, but she is a combination of rook and bishop)
			for (Square pa_square = A8; pa_square <= H1; ++pa_square)
			{
				if (pseudo_attacks[BISHOP][s] & pa_square)
				{
					point_to_point_in_line_bb[s][pa_square] =
						(attacks_bb_by<BISHOP>(s, 0ULL) &
							attacks_bb_by<BISHOP>(pa_square, 0ULL)) | s |
						pa_square;

					between_points_bb[s][pa_square] =
						(attacks_bb_by<BISHOP>(s, square_to_BB(pa_square)) &
							attacks_bb_by<BISHOP>(pa_square, square_to_BB(s)));
				}

				if (pseudo_attacks[ROOK][s] & pa_square)
				{
					point_to_point_in_line_bb[s][pa_square] =
						(attacks_bb_by<ROOK>(s, 0ULL) &
							attacks_bb_by<ROOK>(pa_square, 0ULL)) | s |
						pa_square;

					between_points_bb[s][pa_square] =
						(attacks_bb_by<ROOK>(s, square_to_BB(pa_square)) &
							attacks_bb_by<ROOK>(pa_square, square_to_BB(s)));
				}

				between_points_bb[s][pa_square] |= pa_square;
			}
		}
	}

	/// <summary>
	/// Computes all the bishop and rook attacks.
	/// Here is used fancy magic bitboard approeach
	/// </summary>
	/// <param name="pt">Piece type</param>
	/// <param name="magics">Piece's magic table</param>
	void init_magics(PieceType pt, SMagic magics[])
	{
		assert((pt == ROOK) || (pt == BISHOP));

		U64 subset{}, magic_index{};

		for (Square s = A8; s <= H1; ++s)
		{
			SMagic& sm = magics[s];

			sm.mask = sliding_attacks(pt, s, 0) & ~board_edges(s);
			sm.magic = pt == BISHOP ? BISHOP_MAGIC_NUMBERS[s] : ROOK_MAGIC_NUMBERS[s];
			sm.shift = pt == BISHOP ? RELEVANT_BISHOP_BITS[s] : RELEVANT_ROOK_BITS[s];

			// Using Carry-Rippler trick to enumerate through all subsets of the mask
			subset = 0;
			do
			{
				magic_index = subset;
				magic_index *= sm.magic;
				magic_index >>= 64 - sm.shift;

				( // pick between rook or bishop tables
					pt == BISHOP
					? mBishopAttacks[s][magic_index]
					: mRookAttacks[s][magic_index]
					) = sliding_attacks(pt, s, subset);

				// Steps to derive the expression for enumerating the subsets in the set 'mask'
				// First we set all 'unused' bits of the set OR-ing the one's complement of 'mask'
				// The increment ripples a possible carry through all unused bits
				// and then clear all the unused bits by AND-ing (intersection) the set 'mask'

				// subset = ((subset | ~mask) + 1) & mask
				// replace OR with + and make one's complement to two's complement minus one

				// replacing OR with + works because all unsued bits are always 0
				// subset = (subset + (-mask - 1) + 1) & mask

				// simplify and we should get this expression
				subset = (subset - sm.mask) & sm.mask;
			} while (subset);
		}
	}

	U64 sliding_attacks(PieceType pt, Square s, U64 occ)
	{
		assert((pt == ROOK) || (pt == BISHOP));

		U64 attacks = 0ULL;
		Direction rook_dirs[4] = { DOWN, LEFT, RIGHT, UP };
		Direction bishop_dirs[4] = { DOWN_LEFT, DOWN_RIGHT, UP_LEFT, UP_RIGHT };

		for (Direction d : (pt == ROOK ? rook_dirs : bishop_dirs))
		{
			Square source = s;

			while (is_square_ok(source + d) && square_distance[source][(source + d)] <= 2)
			{
				source += d;
				attacks |= source;

				if (occ & source) break;
			}
		}

		return attacks;
	}

	// Calculate the bishop magic attacks
	// here fancy magic bitboard is used
	U64 bishopAttacks(U64 occ, Square sq)
	{
		occ &= bishop_magic_tbl[sq].mask;
		occ *= bishop_magic_tbl[sq].magic;
		occ >>= 64 - RELEVANT_BISHOP_BITS[sq];

		return mBishopAttacks[sq][occ];
	}

	U64 rookAttacks(U64 occ, Square sq)
	{
		occ &= rook_magic_tbl[sq].mask;
		occ *= rook_magic_tbl[sq].magic;
		occ >>= 64 - RELEVANT_ROOK_BITS[sq];

		return mRookAttacks[sq][occ];
	}

	namespace {
		U64 knight_attacks_mask(const Square& square)
		{
			// result attacks
			U64 attacks = 0ULL;

			// piece bitboard
			U64 bitboard = 0ULL;

			// set piece on board
			set_bit(bitboard, square);

			// generate attacks
			if ((bitboard >> 17) & not_H)  attacks |= (bitboard >> 17); // up and left
			if ((bitboard >> 15) & not_A)  attacks |= (bitboard >> 15); // up and right
			if ((bitboard >> 10) & not_HG) attacks |= (bitboard >> 10); // left
			if ((bitboard >> 6) & not_AB)  attacks |= (bitboard >> 6);  // right

			// flip the offset
			if ((bitboard << 17) & not_A)  attacks |= (bitboard << 17); // down and left
			if ((bitboard << 15) & not_H)  attacks |= (bitboard << 15); // down and right
			if ((bitboard << 10) & not_AB) attacks |= (bitboard << 10); // left
			if ((bitboard << 6) & not_HG)   attacks |= (bitboard << 6); // right

			return attacks;
		}

		U64 king_attacks_mask(const Square& square)
		{
			// result attacks
			U64 attacks = 0ULL;

			// piece bitboard
			U64 bitboard = 0ULL;

			// set piece on board
			set_bit(bitboard, square);

			// generate king attacks
			if (bitboard >> 8)				attacks |= (bitboard >> 8); // upwards
			if ((bitboard >> 9) & not_H)	attacks |= (bitboard >> 9); // up left
			if ((bitboard >> 7) & not_A)	attacks |= (bitboard >> 7); // up right
			if ((bitboard >> 1) & not_H)	attacks |= (bitboard >> 1); // direct left

			if (bitboard << 8)				attacks |= (bitboard << 8); // upwards
			if ((bitboard << 9) & not_A)	attacks |= (bitboard << 9); // up left
			if ((bitboard << 7) & not_H)	attacks |= (bitboard << 7); // up right
			if ((bitboard << 1) & not_A)	attacks |= (bitboard << 1); // direct left

			return attacks;
		}
	}
} // namespace ChessEngine