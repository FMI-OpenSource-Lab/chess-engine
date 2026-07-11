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
#include "zobrist.h"
#include "tt.h"

using namespace KhaosChess;

int main() {
    Bitboards::init();
    BitBase::init();
    Endgames::init();
    Zobrist::init();
    tt::TT.resize(64);

    // Start the UCI loop
    uci_loop();

    // system("pause");
    return 0;
}
