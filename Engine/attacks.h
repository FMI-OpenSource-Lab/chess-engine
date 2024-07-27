#pragma once
#ifndef ATTACKS_H
#define ATTACKS_H

#include <array>

#include "bitboard.h"

using Bitboard = U64;

// Prototypes

// mask attacks
inline Bitboard maskPawnAttacks(Color color, Square square);
inline Bitboard maskKnightAttacks(Square square);
inline Bitboard maskBishopAttacks(Square square);
inline Bitboard maskRookAttacks(Square square);
inline Bitboard maskKingAttacks(Square square);

// generate attacks
inline Bitboard generateBishopAttacks(Square square, Bitboard blockPiece);
inline Bitboard generateRookAttacks(Square square, Bitboard blockPiece);

/* --------------- Mask attacks-------------------- */

inline Bitboard maskPawnAttacks(Color color, Square square)
{
	// result attacks
	Bitboard attacks = 0ULL;

	// piece bitboard
	Bitboard bitboard = 0ULL;

	// set piece on board
	set_bit(bitboard, square);

	// white
	if (!color)
	{
		// generate attacks
		if ((bitboard >> 7) & not_A) attacks |= (bitboard >> 7);
		if ((bitboard >> 9) & not_H) attacks |= (bitboard >> 9);
	}
	else // black
	{
		// generate attacks
		if ((bitboard << 7) & not_HG) attacks |= (bitboard << 7);
		if ((bitboard << 9) & not_AB) attacks |= (bitboard << 9);
	}

	return attacks;
}

inline Bitboard maskKnightAttacks(Square square)
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
inline Bitboard maskBishopAttacks(Square square)
{
	Bitboard attacks = 0ULL;

	int rank, file;

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
inline Bitboard maskRookAttacks(Square square)
{
	Bitboard attacks = 0ULL;

	int rank, file;

	int targetRank = square / 8;
	int targetFile = square % 8;

	// mask relevant rook ocupancy bits
	for (rank = targetRank + 1; rank <= 6; rank++) attacks |= (1ULL << (rank * 8 + targetFile));
	for (rank = targetRank - 1; rank >= 1; rank--) attacks |= (1ULL << (rank * 8 + targetFile));
	for (file = targetFile + 1; file <= 6; file++) attacks |= (1ULL << (targetRank * 8 + file));
	for (file = targetFile - 1; file >= 1; file--) attacks |= (1ULL << (targetRank * 8 + file));

	return attacks;
}

inline Bitboard maskKingAttacks(Square square)
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
inline Bitboard generateBishopAttacks(Square square, Bitboard blockPiece)
{
	Bitboard attacks = 0ULL;

	int rank, file;

	int targetRank = square / 8;
	int targetFile = square % 8;

	// generate bishop attacks
	for (rank = targetRank + 1, file = targetFile + 1; rank <= 7 && file <= 7; rank++, file++)
	{
		attacks |= (1ULL << (rank * 8 + file));

		if (1ULL << (rank * 8 + file) & blockPiece)
			break;
	}

	for (rank = targetRank - 1, file = targetFile + 1; (1ULL >> (rank * 8 + file) & blockPiece), rank >= 0 && file <= 7; rank--, file++)
	{
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
inline Bitboard generateRookAttacks(Square square, Bitboard blockPiece)
{
	Bitboard attacks = 0ULL;

	int rank, file;

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


constexpr inline void initAttacks()
{
	for (Square sq = A8; sq <= H1; ++sq)
	{
		pawnAttacks[WHITE][sq] = maskPawnAttacks(WHITE, sq);
		pawnAttacks[BLACK][sq] = maskPawnAttacks(BLACK, sq);

		knightAttacks[sq] = maskKnightAttacks(sq);

		kingAttacks[sq] = maskKingAttacks(sq);
	}
}

#endif // !ATTACKS_H
