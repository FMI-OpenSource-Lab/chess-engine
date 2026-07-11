#pragma once

#include <string>

#include "bitboard.h"
#include "endgame.h"
#include "position.h"
#include "zobrist.h"

namespace KhaosChess {

// Attack tables, bitbases and Zobrist keys are global state shared by all
// suites; they must be initialized exactly once per test binary
inline void init_engine_once() {
    static const bool initialized = [] {
        Bitboards::init();
        BitBase::init();
        Endgames::init();
        Zobrist::init();
        return true;
    }();
    (void)initialized;
}

// Well-known positions shared across test binaries
inline const std::string kStartPos =
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
inline const std::string kKiwipete =
    "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
inline const std::string kEnPassantPins =
    "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1";
inline const std::string kPromotions =
    "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1";
inline const std::string kTalkchess =
    "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8";

}  // namespace KhaosChess
