#!/usr/bin/env python3
"""Extract Texel training positions from fastchess match PGNs.

Output (stdout): one line per position, "<fen>;<result>" with the game
result from White's perspective (1.0 / 0.5 / 0.0).

Filters:
- only games with a real Result header (1-0, 0-1, 1/2-1/2)
- skip the opening-book phase (first SKIP_PLIES plies): correlated across games
- skip the last END_TRIM plies (mate flailing / adjudication noise)
- skip positions where the side to move is in check (not quiet)
- skip positions where the move actually played was a capture or promotion
  (cheap quietness proxy until we can qsearch-filter with the engine)
- dedupe on FEN minus move counters
"""
import sys
import glob
import chess
import chess.pgn

SKIP_PLIES = 16
END_TRIM = 6
RESULTS = {"1-0": 1.0, "0-1": 0.0, "1/2-1/2": 0.5}


def main(patterns):
    seen = set()
    kept = 0
    games = 0
    files = []
    for p in patterns:
        files.extend(sorted(glob.glob(p)))
    for path in files:
        with open(path, errors="replace") as f:
            while True:
                game = chess.pgn.read_game(f)
                if game is None:
                    break
                result = RESULTS.get(game.headers.get("Result", "*"))
                if result is None:
                    continue
                games += 1
                moves = list(game.mainline_moves())
                board = game.board()
                for i, mv in enumerate(moves):
                    if (
                        SKIP_PLIES <= i < len(moves) - END_TRIM
                        and not board.is_check()
                        and not board.is_capture(mv)
                        and mv.promotion is None
                    ):
                        key = " ".join(board.fen().split()[:4])
                        if key not in seen:
                            seen.add(key)
                            print(f"{board.fen()};{result}")
                            kept += 1
                    board.push(mv)
    print(f"{games} games -> {kept} unique quiet positions", file=sys.stderr)


if __name__ == "__main__":
    main(sys.argv[1:])
