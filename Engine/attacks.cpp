#include "defs.h"
#include "consts.h"
#include "attacks.h"

Attacks::Attacks()
{
	initAttacks();
}

U64 Attacks::maskPawnAttacks(int side, int square)
{
	// result attacks
	U64 attacks = 0ULL;

	// piece bitboard
	U64 bitboard = 0ULL;

	// set piece on board
	set_bit(bitboard, square);

	// white
	if (!side)
	{
		// generate attacks
		if ((bitboard >> 7) & AFile_0) attacks |= (bitboard >> 7);
		if ((bitboard >> 9) & HFile_0) attacks |= (bitboard >> 9);
	}
	else // black
	{
		// generate attacks
		if ((bitboard << 7) & HFile_0) attacks |= (bitboard << 7);
		if ((bitboard << 9) & AFile_0) attacks |= (bitboard << 9);
	}

	return attacks;
}

U64 Attacks::maskKnightAttacks(int square)
{
	// result attacks
	U64 attacks = 0ULL;

	// piece bitboard
	U64 bitboard = 0ULL;

	// set piece on board
	set_bit(bitboard, square);

	// generate attacks
	if ((bitboard >> 17) & HFile_0)  attacks |= (bitboard >> 17); // up and left
	if ((bitboard >> 15) & AFile_0)  attacks |= (bitboard >> 15); // up and right
	if ((bitboard >> 10) & HGFile_0) attacks |= (bitboard >> 10); // left
	if ((bitboard >> 6) & ABFile_0)  attacks |= (bitboard >> 6);  // right

	// flip the offset
	if ((bitboard << 17) & AFile_0)  attacks |= (bitboard << 17); // down and left
	if ((bitboard << 15) & HFile_0)  attacks |= (bitboard << 15); // down and right
	if ((bitboard << 10) & ABFile_0) attacks |= (bitboard << 10); // left
	if ((bitboard << 6) & HGFile_0)   attacks |= (bitboard << 6); // right

	return attacks;
}

// mask bishop attacks
U64 Attacks::maskBishopAttacks(int square)
{
	U64 attacks = 0ULL;

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
U64 Attacks::maskRookAttacks(int square)
{
	U64 attacks = 0ULL;

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

U64 Attacks::maskKingAttacks(int square)
{
	// result attacks
	U64 attacks = 0ULL;

	// piece bitboard
	U64 bitboard = 0ULL;

	// set piece on board
	set_bit(bitboard, square);

	// generate king attacks
	if (bitboard >> 8)				attacks |= (bitboard >> 8); // upwards
	if ((bitboard >> 9) & HFile_0)	attacks |= (bitboard >> 9); // up left
	if ((bitboard >> 9) & AFile_0)	attacks |= (bitboard >> 7); // up right
	if ((bitboard >> 1) & HFile_0)	attacks |= (bitboard >> 1); // direct left

	if (bitboard << 8)				attacks |= (bitboard << 8); // upwards
	if ((bitboard << 9) & AFile_0)	attacks |= (bitboard << 9); // up left
	if ((bitboard << 9) & HFile_0)	attacks |= (bitboard << 7); // up right
	if ((bitboard << 1) & AFile_0)	attacks |= (bitboard << 1); // direct left

	return attacks;
}

// Generating attacks

// generate bishop attacks
U64 Attacks::generateBishopAttacks(int square, U64 blockPiece)
{
	U64 attacks = 0ULL;

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
U64 Attacks::generateRookAttacks(int square, U64 blockPiece)
{
	U64 attacks = 0ULL;

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
}

void Attacks::initAttacks()
{
	for (int sq = 0; sq < 64; sq++)
	{
		pawnAttacks[white][sq] = maskPawnAttacks(white, sq);
		pawnAttacks[black][sq] = maskPawnAttacks(black, sq);

		knightAttacks[sq] = maskKnightAttacks(sq);

		kingAttacks[sq] = maskKingAttacks(sq);
	}
}
