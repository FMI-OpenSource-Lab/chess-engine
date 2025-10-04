#pragma once

#include <chrono>
#include <fstream>
#include <iomanip>
#include <string>

#include "defs.h"
#include "move.h"
#include "movegen.h"
#include "position.h"

namespace KhaosChess {
inline std::uint64_t perft_driver(Position &pos, std::int32_t depth) {
  if (depth == 0) return 1ULL;

  BITBOARD nodes = 0;

  for (const auto &m : MoveList<GT_LEGAL>(pos)) {
    MoveInfo move_info;
    pos.do_move(m, move_info);

    BITBOARD count = perft_driver(pos, depth - 1);

    nodes += count;

    pos.undo_move(m);
  }

  return nodes;
}

inline void perft_debug(Position &pos, std::int32_t depth) {
  BITBOARD nodes = 0;
  auto start_time = std::chrono::high_resolution_clock::now();

  for (const auto &m : MoveList<GT_LEGAL>(pos)) {
    MoveInfo move_info;
    pos.do_move(m, move_info);

    BITBOARD count = perft_driver(pos, depth - 1);

    pos.undo_move(m);

    std::cout << m.uci_move() << ": " << count << std::endl;

    nodes += count;
  }

  auto end_time = std::chrono::high_resolution_clock::now();

  std::cout << "\nNodes: " << nodes;
  std::cout << "\nDepth: " << depth;
  std::cout << "\nTime: "
            << std::chrono::duration_cast<std::chrono::milliseconds>(end_time -
                                                                     start_time)
                   .count()
            << "ms\n";
}

inline void print_perft_table(Position &pos) {
  std::int32_t depth;
  std::cout << "Depth: ";
  std::cin >> depth;

  std::cout << "\n\nDepth";
  std::cout << std::setw(20) << "Nodes";
  std::cout << std::setw(10) << "Time\n";
  std::cout << "--------------------------------------\n";

  for (std::int32_t i = 0; i <= depth; i++) {
    auto start_time = std::chrono::high_resolution_clock::now();

    std::uint64_t nodes = perft_driver(pos, i);

    auto end_time = std::chrono::high_resolution_clock::now();
    auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(
                            end_time - start_time)
                            .count();

    std::cout << i << std::setw(24) << nodes;
    std::cout << std::setw(10) << elapsed_time << " ms\n";
  }
}
}  // namespace KhaosChess
