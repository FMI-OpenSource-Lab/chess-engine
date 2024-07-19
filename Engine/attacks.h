#pragma once
#ifndef ATTACKS_H
#define ATTACKS_H

#include "defs.h"

class Attacks
{
public:
    // Constructor to initialize attack tables
    Attacks();

    // define pawn attacks table [side][square]
    U64 pawnAttacks[2][64]; // 2 - sides to play, 64 - squares on a table

    // define knight attacks table [square]
    U64 knightAttacks[64];

    // define king attack table [square]
    U64 kingAttacks[64];

    // mask attacks
    U64 maskPawnAttacks(int side, int square);
    U64 maskKnightAttacks(int square);
    U64 maskKingAttacks(int square);
    U64 maskBishopAttacks(int square);
    U64 maskRookAttacks(int square);

    // generate attacks
    U64 generateBishopAttacks(int square, U64 blockPiece);
    U64 generateRookAttacks(int square, U64 blockPiece);

    void initAttacks();
};

#endif // !ATTACKS_H
