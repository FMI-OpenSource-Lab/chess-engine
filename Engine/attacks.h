#pragma once
#ifndef ATTACKS_H
#define ATTACKS_H

#include "bitboard.h"
#include "position.h"

#include <array>

using Bitboard = U64;
// Prototypes

namespace Attacks
{
	void init();
};

// pawn attacks table [side][square]
static U64 pawnAttacks[2][64]; // 2 - sides to play, 64 - squares on a table

// knight attacks table [square]
static U64 knightAttacks[64];

// king attack table [square]
static U64 kingAttacks[64];

// mask attacks
extern constexpr Bitboard pawn_attacks_mask(const Color& color, const Square& square);
extern constexpr Bitboard knight_attacks_mask(const Square& square);
extern constexpr Bitboard bishop_attacks_mask(const Square& square);
extern constexpr Bitboard rook_attacks_mask(const Square& square);
extern constexpr Bitboard king_attacks_mask(const Square& square);

// generate attacks
extern constexpr Bitboard bishop_attacks_generate(const Square& square, const Bitboard& blockPiece);
extern constexpr Bitboard rook_attacks_generate(const Square& square, const Bitboard& blockPiece);

void initAttacks(U64** pawnAttacks, U64* knightAttacks, U64* kingAttacks);

#endif // !ATTACKS_H
