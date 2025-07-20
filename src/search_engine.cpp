#include "search_engine.h"

namespace KhaosChess
{
    SearchEngine::SearchEngine(Position &pos)
        : pos(pos),
          should_stop(false),
          max_time(std::chrono::milliseconds(3000)) // Default 3 seconds
    {
    }

    Value SearchEngine::search(int depth, SearchInfo &info)
    {
        // Reset search info
        info = SearchInfo();
        info.depth = depth;
        start_time = std::chrono::high_resolution_clock::now();
        should_stop = false;

        std::vector<Move> pv;
        Value alpha = -VALUE_INFINITE;
        Value beta = VALUE_INFINITE;

        // Iterative deepening
        for (int current_depth = 1; current_depth <= depth; ++current_depth)
        {
            std::vector<Move> current_pv;
            Value score = negamax(current_depth, alpha, beta, info, current_pv);

            // Check if search was stopped early
            if (should_stop && current_depth > 1)
                break;

            // Update PV line if search completed successfully
            if (!should_stop)
                info.pv = current_pv;

            // Update search time
            auto current_time = std::chrono::high_resolution_clock::now();
            info.time = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time);
        }

        return Scorer<SC_ALL>().get_score(pos);
    }

    Value SearchEngine::negamax(int depth, Value alpha, Value beta, SearchInfo &info, std::vector<Move> &pv)
    {
        pv.clear();
        Value score = Scorer<SC_ALL>().get_score(pos);

        // Check if we're out of time
        if (is_time_up())
        {
            info.stopped = should_stop = true;

            return score;
        }

        // Increment node counter
        info.nodes++;

        // Check for draw
        if (score == VALUE_DRAW)
            return VALUE_DRAW;

        // At depth 0, switch to quiescence search
        if (depth <= 0)
            return quiescence(alpha, beta, info);

        // Generate legal moves
        MoveList<GT_LEGAL> moves(pos);

        // If no legal moves, check for checkmate or stalemate
        if (moves.size() == 0)
        {
            Color stm = pos.side_to_move();
            bool is_check = pos.get_attackers_to(pos.square<KING>(stm)) & pos.get_pieces_bb(~stm);

            return -VALUE_MATE * is_check + VALUE_ZERO * (1 - is_check); // Checkmate or Stalemate
        }

        // Score moves for better ordering
        score_moves(moves.begin(), moves.end());

        // Local PV line for this level
        std::vector<Move> local_pv;
        bool found_pv = false;

        // Loop through all moves
        for (const ScoredMoves &scored_move : moves)
        {
            Move move = scored_move; // Implicit conversion from ScoredMoves to Move
            MoveInfo move_info;

            // Make the move
            pos.do_move(move, move_info);

            std::vector<Move> child_pv;
            Value score;

            // Call negamax recursively with negated bounds
            score = -negamax(depth - 1, -beta, -alpha, info, child_pv);

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
            {
                alpha = score;
                found_pv = true;

                // Update the principal variation
                update_pv(local_pv, move, child_pv);
            }
        }

        // If we found a PV, update the caller's PV
        if (found_pv)
            pv = local_pv;

        return alpha;
    }

    Value SearchEngine::quiescence(Value alpha, Value beta, SearchInfo &info)
    {
        // Get the static evaluation
        Value stand_pat = Scorer<SC_ALL>().get_score(pos);

        // Check if we're out of time
        if (is_time_up())
        {
            info.stopped = should_stop = true;

            return stand_pat;
        }

        // Increment quiescence node counter
        info.qnodes++;

        // Stand-pat cutoff
        if (stand_pat >= beta)
            return beta;

        // Update alpha if the static eval is better
        if (stand_pat > alpha)
            alpha = stand_pat;

        // Generate only capture moves
        MoveList<GT_LEGAL> legals(pos);

        // Score legal moves for better ordering
        score_moves(legals.begin(), legals.end());

        // Loop through capture moves
        for (const ScoredMoves &scored_move : legals)
        {
            if (!is_capture(scored_move))
                continue;

            Move move = scored_move; // Implicit conversion from ScoredMoves to Move
            MoveInfo move_info;

            // Make the move
            pos.do_move(move, move_info);

            // Recursively search with negated bounds
            Value score = -quiescence(-beta, -alpha, info);

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

    void SearchEngine::score_moves(ScoredMoves *begin, ScoredMoves *end)
    {
        Value base = Scorer<SC_ALL>().get_score(pos);

        // Simple move ordering: MVV-LVA (Most Valuable Victim - Least Valuable Aggressor)
        for (ScoredMoves *it = begin; it != end; ++it)
        {
            Move move = *it;

            // Score captures based on MVV-LVA (victim value * 10 - aggressor value)
            if (is_capture(move))
            {
                PieceType victim = type_of_piece(pos.get_piece_on(move.source_square()));
                PieceType aggressor = type_of_piece(pos.get_piece_on(move.target_square()));

                // Use material values from your score.h
                it->score = 1000 + MATERIAL_SCORES.piece_value[victim].mg - MATERIAL_SCORES.piece_value[aggressor].mg / 10;

                // Bonus for promotions
                if (move.promoted() != NO_PIECE_TYPE)
                    it->score += 900 + MATERIAL_SCORES.piece_value[move.promoted()].mg;
            }
            // Score quiet moves (can be expanded later)
            else
            {
                // Killers and history heuristic would go here
                // For now, we'll just have a simple baseline
                MoveInfo mi;
                pos.do_move(move, mi);

                Value child_eval = Scorer<SC_ALL>().get_score(pos);

                pos.undo_move(move);
                it->score = child_eval - base;
            }
        }

        // Sort moves based on score (higher scores first)
        std::sort(begin, end, [](auto &a, auto &b)
                  { return a.score > b.score; });
    }

    void SearchEngine::update_pv(std::vector<Move> &pv, Move move, const std::vector<Move> &child_pv)
    {
        pv.clear();
        pv.push_back(move);
        pv.insert(pv.end(), child_pv.begin(), child_pv.end());
    }

    bool SearchEngine::is_capture(Move move)
    {
        return pos.get_piece_on(move.source_square()) != NO_PIECE || move.move_type() == MT_EN_PASSANT;
    }

    bool SearchEngine::is_time_up()
    {
        auto current_time = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time);
        return elapsed >= max_time;
    }

    void SearchEngine::set_max_time(std::chrono::milliseconds max_time)
    {
        this->max_time = max_time;
    }

    // Called from parse_go(…) when you want to do a depth‐limited search

} // namespace KhaosChess
