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

    if (DEBUG_MODE) {
        std::cout << "Debugging\n" << std::endl;

        Position pos{};
        InfoListPtr infos(new std::deque<MoveInfo>(1));

        // pin position: "3r4/bk6/8/r1pPK3/8/8/6B1/8 w - - 0 1";

        pos.set(TEST_FEN, &infos->back());
        std::cout << pos;

        search_position(pos, 5);
    } else
        uci_loop();

    // system("pause");
    return 0;
}
