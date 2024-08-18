#pragma once
#ifndef ATTACKS_H
#define ATTACKS_H

#include "bitboard.h"

#include<vector>
#include <memory>

namespace ChessEngine
{
	using Bitboard = U64;
	// Prototypes

	namespace Attacks
	{
		void init();
	};

	// pawn attacks table [side][square]
	extern Bitboard pawnAttacks[2][64]; // 2 - sides to play, 64 - squares on a table

	// knight attacks table [square]
	extern Bitboard knightAttacks[64];

	// king attack table [square]
	extern Bitboard kingAttacks[64];

	// print attacked squares
	extern void print_attacked_squares(Color color);

	// check if square is attacked
	extern bool is_square_attacked(const Square& square, const Color color);

	// mask attacks
	extern constexpr Bitboard pawn_attacks_mask(const Color& color, const Square& square);
	extern constexpr Bitboard knight_attacks_mask(const Square& square);
	extern constexpr Bitboard bishop_attacks_mask(const Square& square);
	extern constexpr Bitboard rook_attacks_mask(const Square& square);
	extern constexpr Bitboard king_attacks_mask(const Square& square);

	// generate attacks
	extern constexpr Bitboard bishop_attacks_generate(const Square& square, const Bitboard& blockPiece);
	extern constexpr Bitboard rook_attacks_generate(const Square& square, const Bitboard& blockPiece);
}
#endif // !ATTACKS_H
