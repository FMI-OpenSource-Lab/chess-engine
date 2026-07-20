#include "tt.h"

#include <algorithm>

namespace KhaosChess {
namespace tt {

TranspositionTable TT;

void TranspositionTable::resize(std::size_t mb) {
    const std::size_t count = mb * 1024 * 1024 / sizeof(Cluster);

    // round down to a power of two so that (key & mask) is a valid index
    std::size_t pow2 = 1;
    while (pow2 * 2 <= count) {
        pow2 *= 2;
    }

    clusters.assign(pow2, Cluster{});
    mask = pow2 - 1;
}

void TranspositionTable::clear() {
    std::fill(clusters.begin(), clusters.end(), Cluster{});
    generation = 0;
}

void TranspositionTable::new_search() {
    generation = (generation + 1) & 0x3F;
}

TTEntry* TranspositionTable::probe(BITBOARD key, bool& is_found) {
    Cluster& c = clusters[key & mask];

    for (TTEntry& e : c.entry) {
        if (e.key == key) {
            // refresh the age so entries the search still visits stay senior
            e.gen_flag =
                static_cast<std::uint8_t>((generation << 2) | (e.gen_flag & 0x3));
            is_found = true;
            return &e;
        }
    }

    is_found = false;
    return nullptr;
}

void TranspositionTable::store(BITBOARD key, Value score, std::int32_t depth,
                               Flag flag, Move move) {
    Cluster& c = clusters[key & mask];

    // Same position already stored: update that entry in place
    TTEntry* victim = nullptr;
    for (TTEntry& e : c.entry) {
        if (e.key == key) {
            victim = &e;
            break;
        }
    }

    if (victim == nullptr) {
        // Evict the least valuable entry: shallower loses to deeper, and
        // every generation of staleness costs eight plies of seniority
        auto value = [this](const TTEntry& e) {
            std::int32_t age = (generation - e.generation()) & 0x3F;
            return static_cast<std::int32_t>(e.depth) - 8 * age;
        };

        victim = &c.entry[0];
        for (TTEntry& e : c.entry) {
            if (value(e) < value(*victim)) {
                victim = &e;
            }
        }
    } else if (move == Move::invalid_move()) {
        move = victim->move;  // a moveless update must not erase a known move
    }

    *victim = {key, score, move, static_cast<std::int8_t>(depth),
               static_cast<std::uint8_t>((generation << 2) |
                                         static_cast<std::uint8_t>(flag))};
}

}  // namespace tt
}  // namespace KhaosChess
