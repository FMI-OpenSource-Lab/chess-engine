#pragma once

#include <cstdint>

#include "defs.h"
#include "position.h"

namespace KhaosChess {
namespace tt {
enum class Flag { F_EXACT = 0, F_LOWER_BOUND = 1, F_UPPER_BOUND = 2 };

struct TTEntry {
  TTEntry() {}
  TTEntry(int64_t score, int32_t depth, Flag flag, Move move)
      : score(score), depth(depth), flag(flag), move(move) {}

  std::int64_t score;
  std::int32_t depth;

  Flag flag;
  Move move;
};

}  // namespace tt

}  // namespace KhaosChess
