#include "zobrist.h"

#include "random.h"

namespace KhaosChess {
namespace Zobrist {

BITBOARD psq[PIECE_NB][SQUARE_TOTAL];
BITBOARD en_passant[FILE_NB];
BITBOARD castling[CASTLING_RIGHT_NB];
BITBOARD side;

// The seed is fixed on purpose: keys must be identical on every run so
// stored hashes (transposition table, test expectations) stay valid
void init() {
    PRNG rng(1070372);

    for (Piece p = WHITE_PAWN; p <= BLACK_KING; ++p)
        for (Square s = A8; s <= H1; ++s)
            psq[p][s] = rng.rand<BITBOARD>();

    for (File f = FILE_A; f <= FILE_H; ++f)
        en_passant[f] = rng.rand<BITBOARD>();

    for (std::int32_t cr = 0; cr < CASTLING_RIGHT_NB; ++cr)
        castling[cr] = rng.rand<BITBOARD>();

    side = rng.rand<BITBOARD>();
}

}  // namespace Zobrist
}  // namespace KhaosChess
