#include "attacks.h"
#include "position.h"

namespace ChessEngine
{
	Bitboard pawnAttacks[2][64];
	Bitboard knightAttacks[64];
	Bitboard kingAttacks[64];

	// check if square is attacked
	extern bool is_square_attacked(const Square& square, const Color color)
	{
		// !color means white

		// This gets the pawn attacks at the white side and square
		// then applies bitwise AND to the opposide piece
		bool is_pawn_attacks = pawnAttacks[!color][square]
			& bitboards[
				!color
					? get_piece('P')
					: get_piece('p')];

		bool is_knight_attacks = knightAttacks[square]
			& bitboards[
				!color
					? get_piece('N')
					: get_piece('n')];

		bool is_bishop_attacks = bishopAttacks(occupancies[BOTH], square)
			& bitboards[
				!color
					? get_piece('B')
					: get_piece('b')];

		bool is_rook_attacks = rookAttacks(occupancies[BOTH], square)
			& bitboards[
				!color
					? get_piece('R')
					: get_piece('r')];

		bool is_queen_attacks = queenAttacks(occupancies[BOTH], square)
			& bitboards[
				!color
					? get_piece('Q')
					: get_piece('q')];

		//// attacked by black
		//if ((color == BLACK) && (w_pawn_mask & bitboards[p]))
		//	return true;

		return is_pawn_attacks || is_knight_attacks || is_bishop_attacks || is_rook_attacks || is_queen_attacks;
	}

	/* --------------- Mask attacks-------------------- */

	extern constexpr U64 pawn_attacks_mask(const Color& color, const Square& square)
	{
		// result attacks
		Bitboard attacks = 0ULL;

		// piece bitboard
		Bitboard bitboard = 0ULL;

		// set piece on board
		set_bit(bitboard, square);

		// white
		if (color == WHITE)
		{
			// generate attacks
			if ((bitboard >> 7) & not_A) attacks |= (bitboard >> 7);
			if ((bitboard >> 9) & not_H) attacks |= (bitboard >> 9);
		}
		else // black
		{
			// generate attacks
			if ((bitboard << 7) & not_H) attacks |= (bitboard << 7);
			if ((bitboard << 9) & not_A) attacks |= (bitboard << 9);
		}

		return attacks;
	}

	extern constexpr Bitboard knight_attacks_mask(const Square& square)
	{
		// result attacks
		Bitboard attacks = 0ULL;

		// piece bitboard
		Bitboard bitboard = 0ULL;

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

	// mask bishop attacks
	extern constexpr Bitboard bishop_attacks_mask(const Square& square)
	{
		Bitboard attacks = 0ULL;

		int rank = 0, file = 0;

		int targetRank = square / 8;
		int targetFile = square % 8;

		// mask relevant bishop bits
		for (rank = targetRank + 1, file = targetFile + 1; rank <= 6 && file <= 6; rank++, file++) attacks |= (1ULL << (rank * 8 + file));
		for (rank = targetRank - 1, file = targetFile + 1; rank >= 1 && file <= 6; rank--, file++) attacks |= (1ULL << (rank * 8 + file));
		for (rank = targetRank + 1, file = targetFile - 1; rank <= 6 && file >= 1; rank++, file--) attacks |= (1ULL << (rank * 8 + file));
		for (rank = targetRank - 1, file = targetFile - 1; rank >= 1 && file >= 1; rank--, file--) attacks |= (1ULL << (rank * 8 + file));

		return attacks;
	}

	// mask bishop attacks
	extern constexpr Bitboard rook_attacks_mask(const Square& square)
	{
		Bitboard attacks = 0ULL;

		int rank = 0, file = 0;

		int targetRank = square / 8;
		int targetFile = square % 8;

		// mask relevant rook ocupancy bits
		for (rank = targetRank + 1; rank <= 6; rank++) attacks |= (1ULL << (rank * 8 + targetFile));
		for (rank = targetRank - 1; rank >= 1; rank--) attacks |= (1ULL << (rank * 8 + targetFile));
		for (file = targetFile + 1; file <= 6; file++) attacks |= (1ULL << (targetRank * 8 + file));
		for (file = targetFile - 1; file >= 1; file--) attacks |= (1ULL << (targetRank * 8 + file));

		return attacks;
	}

	extern constexpr Bitboard king_attacks_mask(const Square& square)
	{
		// result attacks
		Bitboard attacks = 0ULL;

		// piece bitboard
		Bitboard bitboard = 0ULL;

		// set piece on board
		set_bit(bitboard, square);

		// generate king attacks
		if (bitboard >> 8)				attacks |= (bitboard >> 8); // upwards
		if ((bitboard >> 9) & not_H)	attacks |= (bitboard >> 9); // up left
		if ((bitboard >> 9) & not_A)	attacks |= (bitboard >> 7); // up right
		if ((bitboard >> 1) & not_H)	attacks |= (bitboard >> 1); // direct left

		if (bitboard << 8)				attacks |= (bitboard << 8); // upwards
		if ((bitboard << 9) & not_A)	attacks |= (bitboard << 9); // up left
		if ((bitboard << 9) & not_H)	attacks |= (bitboard << 7); // up right
		if ((bitboard << 1) & not_A)	attacks |= (bitboard << 1); // direct left

		return attacks;
	}

	/* --------------- Generating attacks-------------------- */

	// generate bishop attacks
	extern constexpr Bitboard bishop_attacks_generate(const Square& square, const Bitboard& blockPiece)
	{
		Bitboard attacks = 0ULL;

		int rank = 0, file = 0;

		int targetRank = square / 8;
		int targetFile = square % 8;

		// generate bishop attacks
		for (rank = targetRank + 1, file = targetFile + 1; rank <= 7 && file <= 7; rank++, file++)
		{
			attacks |= (1ULL << (rank * 8 + file));

			if (1ULL << (rank * 8 + file) & blockPiece)
				break;
		}

		for (rank = targetRank - 1, file = targetFile + 1; rank >= 0 && file <= 7; rank--, file++)
		{
			if (1ULL >> (rank * 8 + file) & blockPiece)
				break;

			attacks |= (1ULL << (rank * 8 + file));

			if (1ULL << (rank * 8 + file) & blockPiece)
				break;
		}
		for (rank = targetRank + 1, file = targetFile - 1; rank <= 7 && file >= 0; rank++, file--)
		{
			attacks |= (1ULL << (rank * 8 + file));

			if (1ULL << (rank * 8 + file) & blockPiece)
				break;
		}
		for (rank = targetRank - 1, file = targetFile - 1; rank >= 0 && file >= 0; rank--, file--)
		{
			attacks |= (1ULL << (rank * 8 + file));

			if (1ULL << (rank * 8 + file) & blockPiece)
				break;
		}

		return attacks;
	}

	// generate rook attacks
	extern constexpr Bitboard rook_attacks_generate(const Square& square, const Bitboard& blockPiece)
	{
		Bitboard attacks = 0ULL;

		int rank = 0, file = 0;

		int targetRank = square / 8;
		int targetFile = square % 8;

		// generate rook attacks
		for (rank = targetRank + 1; rank <= 7; rank++)
		{
			attacks |= (1ULL << (rank * 8 + targetFile));

			if ((1ULL << (rank * 8 + targetFile)) & blockPiece)
				break;
		}
		for (rank = targetRank - 1; rank >= 0; rank--)
		{
			attacks |= (1ULL << (rank * 8 + targetFile));
			if (1ULL << (rank * 8 + targetFile) & blockPiece) break;
		}
		for (file = targetFile + 1; file <= 7; file++)
		{
			attacks |= (1ULL << (targetRank * 8 + file));
			if (1ULL << (targetRank * 8 + file) & blockPiece) break;
		}
		for (file = targetFile - 1; file >= 0; file--)
		{
			attacks |= (1ULL << (targetRank * 8 + file));
			if (1ULL << (targetRank * 8 + file) & blockPiece) break;
		}

		return attacks;
	};

	void init_attack_tables()
	{
		// initialize attack tables
		for (Square sq = A8; sq <= H1; ++sq)
		{
			pawnAttacks[WHITE][sq] = pawn_attacks_mask(WHITE, sq);
			pawnAttacks[BLACK][sq] = pawn_attacks_mask(BLACK, sq);

			knightAttacks[sq] = knight_attacks_mask(sq);

			kingAttacks[sq] = king_attacks_mask(sq);
		}
	}

	void Attacks::init()
	{
		init_attack_tables();
	}
}