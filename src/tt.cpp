#include "tt.h"

#include <algorithm>

namespace KhaosChess {
namespace tt {

TranspositionTable TT;

void TranspositionTable::resize(std::size_t mb) {
    const std::size_t count = mb * 1024 * 1024 / sizeof(TTEntry);

    // round down to a power of two so that (key & mask) is a valid index
    std::size_t pow2 = 1;
    while (pow2 * 2 <= count) {
        pow2 *= 2;
    }

    entries.assign(pow2, TTEntry{});
    mask = pow2 - 1;
}

void TranspositionTable::clear() {
    std::fill(entries.begin(), entries.end(), TTEntry{});
}

TTEntry* TranspositionTable::probe(BITBOARD key, bool& is_found) {
    TTEntry* e = &entries[key & mask];
    is_found = (e->key == key);

    return e;
}

void TranspositionTable::store(BITBOARD key, Value score, std::int32_t depth, Flag flag,
                               Move move) {
    TTEntry* e = &entries[key & mask];
    *e = {key, score, move, static_cast<std::int8_t>(depth), flag};
}

}  // namespace tt
}  // namespace KhaosChess
