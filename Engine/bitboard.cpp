#include "bitboard.h"
#include "attacks.h"

#include<iostream>

SMagic mBishopTbl[64];
SMagic mRookTbl[64];

extern inline void init_sliders_attacks(PieceType py)
{
    // loop over 64 board squares
    for (int square = 0; square < 64; square++)
    {
        Square sq = static_cast<Square>(square);

        // init bishop & rook masks
        mBishopTbl[square].mask = maskBishopAttacks(sq);
        mRookTbl[square].mask = maskRookAttacks(sq);

        // init current mask
        U64 attack_mask = py == BISHOP ? mBishopTbl[square].mask : mRookTbl[square].mask;
        
        // init relevant occupancy bit count
        int relevant_bits_count = countBits(attack_mask);

        // init occupancy indicies
        int occupancy_indicies = (1 << relevant_bits_count);

        // loop over occupancy indicies
        for (int index = 0; index < occupancy_indicies; index++)
        {
            // bishop
            if (py == BISHOP)
            {
                // init current occupancy variation
                U64 occupancy = set_occupancy(index, relevant_bits_count, attack_mask);

                // init magic index
                int magic_index = (occupancy * BISHOP_MAGIC_NUMBERS[square]) >> (64 - RELEVANT_BISHOP_BITS[square]);
                mBishopTbl[square].magic = BISHOP_MAGIC_NUMBERS[square];

                // init bishop attacks
                mBishopAttacks[square][magic_index] = generateBishopAttacks(sq, occupancy);
            }

            // rook
            else if (py == ROOK)
            {
                // init current occupancy variation
                U64 occupancy = set_occupancy(index, relevant_bits_count, attack_mask);

                // init magic index
                int magic_index = (occupancy * ROOK_MAGIC_NUMBERS[square]) >> (64 - RELEVANT_ROOK_BITS[square]);
                mRookTbl[square].magic = ROOK_MAGIC_NUMBERS[square];

                // init bishop attacks
                mRookAttacks[square][magic_index] = generateRookAttacks(sq, occupancy);
            }
        }
    }
}

// get bishop attacks assuming current board occupancy
//    occupancy &= bishop_masks[square];
//    occupancy *= BISHOP_MAGIC_NUMBERS[square];
//    occupancy >>= 64 - RELEVANT_BISHOP_BITS[square];
//
//    // return bishop attacks
//    return bishop_attacks[square][occupancy];

extern U64 bishopAttacks(U64 occ, Square sq)
{
	occ &= mBishopTbl[sq].mask;
	occ *= mBishopTbl[sq].magic;
	occ >>= 64 - RELEVANT_BISHOP_BITS[sq];

	return mBishopAttacks[sq][occ];
}

extern U64 rookAttacks(U64 occ, Square sq)
{
    occ &= mRookTbl[sq].mask;
    occ *= mRookTbl[sq].magic;
    occ >>= 64 - RELEVANT_ROOK_BITS[sq];

    return mRookAttacks[sq][occ];
}
