#include <iostream>
#include <numeric>

#include "bitboard.h"
#include "defs.h"
#include "endgame.h"
#include "perft.h"
#include "position.h"
#include "score.h"
#include "search_engine.h"
#include "uci.h"

using namespace KhaosChess;

int main() {
  Bitboards::init();
  BitBase::init();
  Endgames::init();

#ifdef DEBUG_MODE
  std::cout << "Debugging\n" << std::endl;

  Position pos{};
  InfoListPtr infos(new std::deque<MoveInfo>(1));

  // pin position: "3r4/bk6/8/r1pPK3/8/8/6B1/8 w - - 0 1";

  pos.set(TEST_FEN, &infos->back());
  std::cout << pos;

  Value best = 0;
  alpha_beta_max(pos, -VALUE_INFINITE, VALUE_INFINITE, 3, best);

  std::cout << best << "\n";

  return 0;
#endif

  // Start the UCI loop
  uci_loop();

  // system("pause");
  return 0;
}
