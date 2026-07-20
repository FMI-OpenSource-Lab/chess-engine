#pragma once

#include <cstdint>
#include <vector>

#include "defs.h"
#include "move.h"

namespace KhaosChess {
namespace tt {
enum class Flag : std::uint8_t { F_EXACT = 0,
                                 F_LOWER_BOUND = 1,
                                 F_UPPER_BOUND = 2 };

// The bound flag lives in the low 2 bits of gen_flag, the search
// generation in the high 6 bits, so an entry knows both what its score
// means and how stale it is.
struct TTEntry {
    BITBOARD key = 0;
    Value score = 0;
    Move move;
    std::int8_t depth = -1;
    std::uint8_t gen_flag = 0;

    Flag flag() const {
        return static_cast<Flag>(gen_flag & 0x3);
    }
    std::uint8_t generation() const {
        return gen_flag >> 2;
    }
};

static_assert(sizeof(TTEntry) == 16, "TTEntry should pack to 16 bytes");

// Four entries share one cache line; a store evicts the least valuable
// of the four (shallowest, oldest) instead of whatever the key hashed onto
struct alignas(64) Cluster {
    TTEntry entry[4];
};

static_assert(sizeof(Cluster) == 64, "Cluster should fill one cache line");

class TranspositionTable {
   public:
    void resize(std::size_t mb);
    void clear();

    // Bump the generation; call once at the start of every search
    void new_search();

    // Returns the matching entry (age refreshed) when found, otherwise
    // nullptr - callers must check `found` before dereferencing
    TTEntry* probe(BITBOARD key, bool& found);
    void store(BITBOARD key, Value score, std::int32_t depth, Flag flag,
               Move move);

   private:
    std::vector<Cluster> clusters;
    std::size_t mask = 0;         // index = key & mask, so size must be 2^n
    std::uint8_t generation = 0;  // 6 bits, wraps around
};

extern TranspositionTable TT;

}  // namespace tt

}  // namespace KhaosChess
