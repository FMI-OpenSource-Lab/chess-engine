#pragma once

#include "defs.h"

namespace KhaosChess {

// Zobrist hashing: every position maps to a 64-bit key built by XORing one
// random number per board feature (piece on square, en passant file,
// castling rights, side to move). Position::do_move updates the key
// incrementally, exploiting that XOR is its own inverse
namespace Zobrist {
extern BITBOARD psq[PIECE_NB][SQUARE_TOTAL];  // piece on square
extern BITBOARD en_passant[FILE_NB];          // ep file (only file matters)
extern BITBOARD castling[CASTLING_RIGHT_NB];  // indexed by full rights mask
extern BITBOARD side;                         // xored when black to move

void init();
}  // namespace Zobrist

}  // namespace KhaosChess
