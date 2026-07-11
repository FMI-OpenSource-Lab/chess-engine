#pragma once

#include <cstdint>

#include "defs.h"
#include "move.h"

namespace KhaosChess {
namespace tt {
enum class Flag : std::uint8_t { F_EXACT = 0,
                                 F_LOWER_BOUND = 1,
                                 F_UPPER_BOUND = 2 };

struct TTEntry {
    BITBOARD key = 0;
    Value score = 0;
    Move move;
    std::int8_t depth = -1;
    Flag flag = Flag::F_EXACT;
};

static_assert(sizeof(TTEntry) == 16, "TTEntry should pack to 16 bytes");

class TranspositionTable {
   public:
    void resize(std::size_t mb);
    void clear();

    TTEntry* probe(BITBOARD key, bool& found);
    void store(BITBOARD key, Value score, std::int32_t depth, Flag flag,
               Move move);

   private:
    std::vector<TTEntry> entries;
    std::size_t mask = 0;  // index = key & mask, so size must be 2^n
};

extern TranspositionTable TT;

}  // namespace tt

}  // namespace KhaosChess
