#include "search_engine.h"

#include <iostream>
#include <cstdlib>
#include <cstring>

#include "tt.h"

namespace KhaosChess {
// Mate scores are ply-from-root; the TT is position-keyed, and the same
// position occurs at different root distances. Entries therefore hold
// mate scores relative to the node, converted on store and probe
static Value score_to_tt(Value score, std::int32_t ply) {
    if (score >= VALUE_MATE - MAX_PLY)
        return score + ply;
    if (score <= -VALUE_MATE + MAX_PLY)
        return score - ply;
    return score;
}

static Value score_from_tt(Value score, std::int32_t ply) {
    if (score >= VALUE_MATE - MAX_PLY)
        return score - ply;
    if (score <= -VALUE_MATE + MAX_PLY)
        return score + ply;
    return score;
}

SearchEngine::SearchEngine(Position& pos)
    : pos(pos),
      max_time(std::chrono::milliseconds(3000)),  // Default 3 seconds
      should_stop(false),
      time_checks(0) {
}

Value SearchEngine::search(std::int32_t depth, SearchInfo& info) {
    // Reset search info
    info = SearchInfo();
    info.depth = depth;
    start_time = std::chrono::high_resolution_clock::now();
    should_stop = false;
    time_checks = 0;

    // Fresh ordering state per search; history persists across the
    // iterative-deepening iterations below, which is where it pays off
    for (std::int32_t i = 0; i < MAX_PLY; ++i)
        killers[i][0] = killers[i][1] = Move::invalid_move();
    std::memset(history, 0, sizeof(history));

    Value alpha = -VALUE_INFINITE;
    Value beta = VALUE_INFINITE;
    Value best_score = -VALUE_INFINITE;

    // Iterative deepening
    for (std::int32_t current_depth = 1; current_depth <= depth;
         ++current_depth) {
        Value score = negamax(current_depth, 0, alpha, beta, info);

        // Check if search was stopped early
        if (should_stop && current_depth > 1)
            break;

        // Update search time
        auto current_time = std::chrono::high_resolution_clock::now();
        info.time = std::chrono::duration_cast<std::chrono::milliseconds>(
            current_time - start_time);

        // Update PV line and report if search completed successfully
        if (!should_stop) {
            info.pv.assign(pv_table[0], pv_table[0] + pv_length[0]);
            best_score = score;

            std::cout << "info depth " << current_depth;

            if (std::abs(score) >= VALUE_MATE - MAX_PLY) {
                std::int32_t mate_in = (VALUE_MATE - std::abs(score));
                std::cout << " score mate " << (score > 0 ? mate_in : -mate_in);
            } else {
                std::cout << " score cp " << score;
            }

            std::cout << " nodes " << (info.nodes + info.q_nodes) << " time "
                      << info.time.count() << " pv";
            for (const Move& m : info.pv)
                std::cout << " " << m.uci_move();
            std::cout << std::endl;
        }
    }

    return best_score;
}

Value SearchEngine::negamax(std::int32_t depth, std::int32_t ply, Value alpha, Value beta,
                            SearchInfo& info) {
    pv_length[ply] = ply;

    // Check if we're out of time
    if (is_time_up()) {
        info.stopped = should_stop = true;

        return Scorer<SC_ALL>().get_score(pos);
    }

    // Increment node counter
    info.nodes++;

    // Draw by repetition or fifty-move rule
    if (ply > 0 && pos.is_draw()) {
        return VALUE_DRAW;
    }

    // Transposition table probe: another move order may already have
    // searched this position at least as deeply
    bool is_tt_hit;
    tt::TTEntry* tte = tt::TT.probe(pos.key(), is_tt_hit);

    if (is_tt_hit && ply > 0 && tte->depth >= depth) {
        Value tt_score = score_from_tt(tte->score, ply);

        if (tte->flag == tt::Flag::F_EXACT ||
            (tte->flag == tt::Flag::F_LOWER_BOUND && tt_score >= beta) ||
            (tte->flag == tt::Flag::F_UPPER_BOUND && tt_score <= alpha))
            return tt_score;
    }

    // At depth 0, switch to quiescence search
    if (depth <= 0)
        return quiescence(ply, alpha, beta, info);

    // Generate legal moves
    MoveList<GT_LEGAL> moves(pos);

    // If no legal moves, check for checkmate or stalemate
    if (moves.size() == 0) {
        Color stm = pos.side_to_move();
        bool is_check =
            pos.get_attackers_to(pos.square<KING>(stm)) & pos.get_pieces_bb(~stm);

        return is_check ? -VALUE_MATE + ply : VALUE_DRAW;  // Checkmate or Stalemate
    }

    // Score moves for better ordering
    score_moves(moves.begin(), moves.end(), ply,
                is_tt_hit ? tte->move : Move::invalid_move());

    // Local PV line for this level
    bool found_pv = false;
    Move best_move = Move::invalid_move();

    // Loop through all moves
    for (const ScoredMoves& scored_move : moves) {
        Move move = scored_move;  // Implicit conversion from ScoredMoves to Move
        MoveInfo move_info;

        // Make the move
        pos.do_move(move, move_info);

        Value score;

        // Call negamax recursively with negated bounds
        score = -negamax(depth - 1, ply + 1, -beta, -alpha, info);

        // Undo the move
        pos.undo_move(move);

        // Check if search was stopped
        if (should_stop)
            return 0;

        // Beta cutoff (fail-high)
        if (score >= beta) {
            // Quiet cutoff
            if (!is_capture(move) && move.promoted() == NO_PIECE_TYPE) {
                if (move != killers[ply][0]) {
                    killers[ply][1] = killers[ply][0];
                    killers[ply][0] = move;
                }

                std::int32_t& h = history[pos.side_to_move()]
                                         [move.source_square()]
                                         [move.target_square()];
                h = std::min(h + depth * depth, 16'000);
            }

            tt::TT.store(pos.key(), score_to_tt(beta, ply), depth,
                         tt::Flag::F_LOWER_BOUND, move);
            return beta;
        }

        // Better move found
        if (score > alpha) {
            alpha = score;
            found_pv = true;
            best_move = move;

            // Update the principal variation
            pv_table[ply][ply] = move;

            for (std::int32_t i = ply + 1; i < pv_length[ply + 1]; ++i) {
                pv_table[ply][i] = pv_table[ply + 1][i];
            }

            pv_length[ply] = pv_length[ply + 1];
        }
    }

    tt::TT.store(pos.key(), score_to_tt(alpha, ply), depth,
                 found_pv ? tt::Flag::F_EXACT : tt::Flag::F_UPPER_BOUND,
                 best_move);

    return alpha;
}

Value SearchEngine::quiescence(std::int32_t ply, Value alpha, Value beta, SearchInfo& info) {
    // Check if we're out of time
    if (is_time_up()) {
        info.stopped = should_stop = true;

        return alpha;  // discarded by callers once should_stop is set
    }

    // Increment quiescence node counter
    info.q_nodes++;

    // Perpetual-check cycles repeat positions without consuming material;
    // without this, evasion recursion runs until the stack overflows
    if (pos.is_draw())
        return VALUE_DRAW;

    // Hard safety net for any other pathologically long sequence
    if (ply >= MAX_PLY)
        return Scorer<SC_ALL>().get_score(pos);

    Color stm = pos.side_to_move();
    bool in_check = pos.get_attackers_to(pos.square<KING>(stm)) & pos.get_pieces_bb(~stm);

    if (!in_check) {
        // Stand-pat: the static eval is a lower bound, since we may stop capturing
        Value stand_pat = Scorer<SC_ALL>().get_score(pos);

        // Stand-pat cutoff
        if (stand_pat >= beta)
            return beta;

        // Update alpha if the static eval is better
        if (stand_pat > alpha)
            alpha = stand_pat;
    }

    // Generate only capture moves
    MoveList<GT_LEGAL> legals(pos);

    // No legal moves while in check is checkmate
    if (in_check && legals.size() == 0) {
        return -VALUE_MATE + ply;
    }

    // Score legal moves for better ordering
    score_moves(legals.begin(), legals.end(), ply, Move::invalid_move(), false);

    // Loop through capture moves
    for (const ScoredMoves& scored_move : legals) {
        if (!in_check && !is_capture(scored_move))
            continue;

        Move move = scored_move;  // Implicit conversion from ScoredMoves to Move
        MoveInfo move_info;

        // Make the move
        pos.do_move(move, move_info);

        // Recursively search with negated bounds
        Value score = -quiescence(ply + 1, -beta, -alpha, info);

        // Undo the move
        pos.undo_move(move);

        // Check if search was stopped
        if (should_stop)
            return 0;

        // Beta cutoff (fail-high)
        if (score >= beta)
            return beta;

        // Better move found
        if (score > alpha)
            alpha = score;
    }

    return alpha;
}

void SearchEngine::score_moves(ScoredMoves* begin, ScoredMoves* end,
                               std::int32_t ply, Move tt_move,
                               bool score_quiets) {
    // Simple move ordering: MVV-LVA (Most Valuable Victim - Least Valuable
    // Aggressor)
    for (ScoredMoves* it = begin; it != end; ++it) {
        Move move = *it;

        // The TT move proved best here before; search it first
        if (move == tt_move) {
            it->score = 1'000'000;
            continue;
        }

        // Captures and promotions: MVV-LVA (most valuable victim first,
        // least valuable aggressor as tiebreak)
        if (is_capture(move) || move.promoted() != NO_PIECE_TYPE) {
            PieceType victim = type_of_piece(pos.get_piece_on(move.target_square()));
            PieceType aggressor =
                type_of_piece(pos.get_piece_on(move.source_square()));

            it->score = 100'000 + MATERIAL_SCORES.piece_value[victim].mg -
                        MATERIAL_SCORES.piece_value[aggressor].mg / 10;

            if (move.promoted() != NO_PIECE_TYPE)
                it->score += 900 + MATERIAL_SCORES.piece_value[move.promoted()].mg;
        }
        // Quiets: killers first, then history
        else if (score_quiets) {
            if (move == killers[ply][0])
                it->score = 90'000;
            else if (move == killers[ply][1])
                it->score = 80'000;
            else
                it->score = history[pos.side_to_move()][move.source_square()]
                                   [move.target_square()];
        } else {
            // Quiescence discards quiets (except evasions)
            it->score = -1'000'000;
        }
    }

    std::sort(begin, end, [](auto& a, auto& b) { return a.score > b.score; });
}

bool SearchEngine::is_capture(Move move) {
    return pos.get_piece_on(move.target_square()) != NO_PIECE ||
           move.move_type() == MT_EN_PASSANT;
}

bool SearchEngine::is_time_up() {
    if (should_stop)
        return true;

    // The clock read is costly; poll it once every 2048 calls
    if ((++time_checks & 2047) != 0)
        return false;

    auto current_time = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        current_time - start_time);
    return elapsed >= max_time;
}

void SearchEngine::set_max_time(std::chrono::milliseconds max_time) {
    this->max_time = max_time;
}

}  // namespace KhaosChess
