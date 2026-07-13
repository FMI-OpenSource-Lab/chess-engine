#include "uci.h"

#include <stdlib.h>

#include <iostream>

#include "move.h"
#include "movegen.h"
#include "perft.h"
#include "position.h"
#include "search_engine.h"
#include "tt.h"

namespace KhaosChess {
Move parse_move(std::string move_string, const Position& pos) {
    Square source =
        Square((move_string[0] - 'a') + (8 - (move_string[1] - '0')) * 8);
    Square target =
        Square((move_string[2] - 'a') + (8 - (move_string[3] - '0')) * 8);

    for (auto& m : MoveList<GT_LEGAL>(pos)) {
        Square m_target = m.target_square();

        // Castling is encoded king->rook square; UCI speaks king->g/c file
        if (m.move_type() == MT_CASTLING)
            m_target = make_square(m_target > m.source_square() ? FILE_G : FILE_C,
                                   rank_of(m.source_square()));

        if (m.source_square() == source && m_target == target) {
            if (m.move_type() == MT_PROMOTION) {
                // UCI promotion letters are always lowercase (e7e8q); BLACK
                // indexes the lowercase half of ascii_pieces
                if (move_string[4] == ascii_pieces[get_piece(BLACK, m.promoted())])
                    return m;

                continue;
            }

            return m;
        }
    }

    return Move::invalid_move();
}

void parse_position(const char* cmd, Position& pos, InfoListPtr& infos) {
    // every position command rebuilds the game from scratch.
    // start a fresh history chain, because old one is dead
    infos->clear();
    infos->emplace_back();

    // shift to next token, because "position" is 8 characters and " " is 1,
    // hence shift 9
    cmd += 9;
    const char* current = cmd;

    if (strncmp(current, "startpos", 8) == 0)
        pos.set(START_FEN, &infos->back());
    else if (strncmp(current, "testpos", 7) == 0)
        pos.set(TEST_FEN, &infos->back());
    else  // UCI "fen" command
    {
        // fen command is available
        current = strstr(cmd, "fen");

        // fen command is not available
        if (current == NULL)
            pos.set(START_FEN, &infos->back());
        else  // found FEN
        {
            current += 4;

            // init board position from FEN command
            pos.set(current, &infos->back());
        }
    }

    // parse moves command
    current = strstr(cmd, "moves");

    // moves available
    if (current != NULL) {
        // shift to the moves
        current += 6;

        while (*current) {
            // parse next move
            Move move = parse_move(current, pos);

            // no move
            if (move == Move::invalid_move())
                break;

            // make the move
            infos->emplace_back();
            pos.do_move(move, infos->back());

            while (*current && *current != ' ')
                current++;

            current++;
        }
    }
}

void parse_go(const char* cmd, Position& pos) {
    const char* current;

    if ((current = strstr(cmd, "perft"))) {
        perft_debug(pos, atoi(current + 6));
        return;
    }

    std::int32_t depth = 0;
    std::int64_t search_time = 0;  // in milliseconds

    // fixed depth search
    if ((current = strstr(cmd, "depth")))
        depth = atoi(current + 6);

    // fixed nodes per move
    std::int64_t max_nodes = 0;
    if ((current = strstr(cmd, "nodes")))
        max_nodes = atoll(current + 6);

    // fixed time per move
    if ((current = strstr(cmd, "movetime")))
        search_time = atoll(current + 9);
    else {
        // allocate a slice of the remaining clock time
        const char* time_token = pos.side_to_move() == WHITE ? "wtime" : "btime";
        const char* inc_token = pos.side_to_move() == WHITE ? "winc" : "binc";

        std::int64_t time_left = 0;
        std::int64_t increment = 0;

        if ((current = strstr(cmd, time_token)))
            time_left = atoll(current + 6);
        if ((current = strstr(cmd, inc_token)))
            increment = atoll(current + 5);

        if (time_left)
            search_time = time_left / 20 + increment / 2;
    }

    // bare "go" or "go infinite": no explicit limit, fall back to a fixed depth
    if (!depth && !search_time && !max_nodes)
        depth = 6;

    SearchEngine engine(pos);

    // when only a depth limit is given, effectively disable the time limit
    engine.set_max_time(std::chrono::milliseconds(
        search_time ? search_time : 24LL * 60 * 60 * 1000));

    if (max_nodes)
        engine.set_max_nodes(static_cast<std::uint64_t>(max_nodes));

    SearchInfo info;
    engine.search(depth ? depth : 64, info);

    Move best = info.pv.empty() ? Move::invalid_move() : info.pv[0];

    // search was stopped before depth 1 completed; play any legal move
    if (best == Move::invalid_move()) {
        MoveList<GT_LEGAL> moves(pos);
        if (moves.size())
            best = *moves.begin();
    }

    if (best == Move::invalid_move())
        std::cout << "bestmove (none)\n";
    else
        std::cout << "bestmove " << best.uci_move() << "\n";
}

/*
        GUI -> isready
        Engine -> readyok
        GUI -> ucinewgame
*/
void uci_loop() {
    constexpr auto INPUT_BUFFER = 10000;

    // rest stdin & stdout buffers
    std::setvbuf(stdin, NULL, _IONBF, 0);
    std::setvbuf(stdout, NULL, _IONBF, 0);

    // def user/GUI inout buffer
    char input_buffer[INPUT_BUFFER];

    std::cout << "id name " << NAME << "\n";
    std::cout << "id author " << AUTHOR << "\n";
    std::cout << "uciok\n";

    InfoListPtr infos(new std::deque<MoveInfo>(1));
    Position pos;

    // Boot into a valid (empty) position so commands like "d" are safe
    // even before the GUI sends "position"
    pos.set(EMPTY_FEN, &infos->back());

    // main loop
    while (true) {
        // rest user/GUI input
        memset(input_buffer, 0, sizeof(input_buffer));

        // making sure output reacehes GUI
        fflush(stdout);

        // get user/GUI input
        if (!fgets(input_buffer, INPUT_BUFFER, stdin))
            break;

        // available input
        if (input_buffer[0] == '\n')
            continue;

        // parse UCI "isready" command
        if (strncmp(input_buffer, "isready", 7) == 0) {
            std::cout << "readyok\n";
            continue;
        }
        // parse UCI "position" command
        else if (strncmp(input_buffer, "position", 8) == 0)
            parse_position(input_buffer, pos, infos);

        // parse UCI "ucinewgame" command
        else if (strncmp(input_buffer, "ucinewgame", 10) == 0) {
            tt::TT.clear();
            parse_position("position startpos", pos, infos);
        }

        // parse UCI "go" command
        else if (strncmp(input_buffer, "go", 2) == 0)
            parse_go(input_buffer, pos);

        // parse UCI "quit" command
        else if (strncmp(input_buffer, "quit", 4) == 0)
            break;  // exit loop

        // parse UCI "uci" command
        else if (strncmp(input_buffer, "uci", 3) == 0) {
            std::cout << "\nid name " << NAME << "\n";
            std::cout << "id author " << AUTHOR << "\n";
            std::cout << "uciok\n";
        }

        else if (!strncmp(input_buffer, "d", 1))
            std::cout << pos << std::endl;
    }
}
}  // namespace KhaosChess
