#pragma once
#ifndef ATTACKS_H
#define ATTACKS_H

#include "bitboard.h"

namespace ChessEngine
{
	using Bitboard = U64;
	// Prototypes

	namespace Attacks
	{
		void init();
	};

	// Initialize the attack tables (pawns, king, knight)
	void init_attack_tables();

	// check if square is attacked
	extern int is_square_attacked(const Square& square, const Color color);

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
