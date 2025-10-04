#include "score.h"

#include <algorithm>
#include <iostream>
#include <memory>

#include "bitboard.h"
#include "consts.h"
#include "defs.h"
#include "move.h"
#include "position.h"

namespace KhaosChess {
namespace {
constexpr BITBOARD opponent_ranks_for(Color c) {
  return c == WHITE ? Rank8_Bits | Rank7_Bits | Rank6_Bits | Rank5_Bits
                    : Rank1_Bits | Rank2_Bits | Rank3_Bits | Rank4_Bits;
}

BITBOARD get_real_possible_moves(const Color c, const Position &pos, Square s,
                                 BITBOARD moves) {
  const Square ksq = pos.square<KING>(c);

  // Get all opponent pieces, except pawns
  BITBOARD opp_pieces = pos.get_pieces_bb(~c) ^ pos.get_pieces_bb(PAWN, ~c);

  if (pos.get_king_blockers(c) & ksq) moves &= in_between_bb(s, ksq);

  // cannot move into square with our piece
  moves &= ~pos.get_pieces_bb(c);

  // Get the other sides's pawn attacks into one bitboard
  BITBOARD opp_pawns = pos.get_pieces_bb(PAWN, ~c);
  BITBOARD opp_pawn_attacks = 0ULL;

  while (opp_pawns)
    opp_pawn_attacks |= pawn_attacks_bb(~c, pop_ls1b(opp_pawns));

  // Don't count squares attacked by opponents pawns with nothing valuable
  // there
  moves &= ~(opp_pawn_attacks & opp_pieces);

  return moves;
}

// Score the pawns by given component and color
// Returns the score for the given component and color
// Does not score pawn shield, this is left for the king safety scoring
template <ScoreComponent Component, Color Us>
Score score_pawns(const Position &pos) {
  constexpr Color Them = ~Us;
  constexpr Direction Up = pawn_push_direction(Us);
  constexpr Direction Down = pawn_push_direction(Them);
  constexpr std::int32_t inc = Us == WHITE ? 1 : -1;

  Score score = 0;

  BITBOARD our_pawns = pos.get_pieces_bb(PAWN, Us);
  BITBOARD opp_pawns = pos.get_pieces_bb(PAWN, Them);

  if (Component == SC_MATERIAL || Component == SC_ALL)
    score += MATERIAL_SCORES.piece_value[PAWN] * Value(pos.count<PAWN>(Us));

  // Return early if we only need material evaluation
  if (Component == SC_MATERIAL) return score;

  // Center controlled squares by side
  BITBOARD center_squares = FileC_Bits | FileD_Bits | FileE_Bits | FileF_Bits;
  center_squares &=
      (Us == WHITE) ? Rank5_Bits | Rank6_Bits : Rank3_Bits | Rank4_Bits;

  BITBOARD pawns = our_pawns;
  while (pawns) {
    // Get the pawn structure components
    if (Component == SC_PAWN_STRUCTURE || Component == SC_ALL) {
      Square s = pop_ls1b(pawns);

      Rank r = rank_of(s);
      Rank rel_r = rank_relative_to_side(Us, r);
      File f = file_of(s);

      const BITBOARD our_attacks = pawn_attacks_bb(Us, s);

      // Gets our pawns located on the adjacent files
      const BITBOARD neighbors = our_pawns & neighboring_files[f];

      // Pawns that located side by side on adjacent files
      const BITBOARD phalanx = neighbors & rank_bb(r);

      // Checks the adjacent rank for supporting neighboring pawns
      const BITBOARD support = neighbors & rank_bb(Rank(r - inc));

      // Checks for opponent pawns that can be captured
      const BITBOARD lever = opp_pawns & our_attacks;

      // Checks for opponent pawns that can be pushed
      const BITBOARD lever_push = opp_pawns & move_to<Up>(our_attacks);

      // Checks for opponent pawns that are opposed to our pawns
      const BITBOARD opposed = opp_pawns & passed_pawn_path<Us>(s);

      const bool are_blocked = opp_pawns & move_to<Up>(square_to_BB(s));
      const bool are_doubled = our_pawns & move_to<Down>(square_to_BB(s));
      const bool is_backward = !(neighbors & passed_pawn_path<Them>(s + Up)) &&
                               (are_blocked || lever_push);
      const bool is_passed = !opposed || !(opposed ^ lever) ||
                             (!(opposed ^ lever_push) &&
                              count_bits(phalanx) >= count_bits(lever_push));

      score += count_bits(our_attacks & center_squares) *
               PAWN_STRUCTURE_SCORES.control_center;

      if (are_doubled) score += PAWN_STRUCTURE_SCORES.doubled;
      if (support | phalanx) {
        score += Score(PAWN_STRUCTURE_SCORES.connected_bonus[rel_r] *
                       (1 + static_cast<std::int32_t>(bool(phalanx)) -
                        static_cast<std::int32_t>(bool(opposed))));
        score += Score(10 * count_bits(support));
      } else if (!neighbors)
        score += PAWN_STRUCTURE_SCORES.isolated;
      else if (is_backward)
        score += PAWN_STRUCTURE_SCORES.backward;

      if (is_passed)
        score += PAWN_STRUCTURE_SCORES.passed *
                 PAWN_STRUCTURE_SCORES.passed_rank_weight[rel_r];
    }
  }

  return score;
}

template <ScoreComponent Component, Color Us>
Score score_knights(const Position &pos) {
  constexpr Color Them = ~Us;
  constexpr BITBOARD center_bits =
      FileC_Bits | FileD_Bits | FileE_Bits | FileF_Bits;
  constexpr BITBOARD opp_ranks = opponent_ranks_for(Us);

  Score score = 0;

  BITBOARD knights = pos.get_pieces_bb(KNIGHT, Us);

  if (Component == SC_MATERIAL || Component == SC_ALL)
    score += MATERIAL_SCORES.piece_value[KNIGHT] * Value(pos.count<KNIGHT>(Us));

  if (Component == SC_MATERIAL) return score;

  while (knights) {
    Square s = pop_ls1b(knights);
    BITBOARD attacking = attacks_bb_by<KNIGHT>(s);

    // Piece coordination
    if (Component == SC_PIECE_COORDINATION || Component == SC_ALL) {
      if (pos.get_attacks_by<PAWN>(Us) & square_to_BB(s))
        score += PIECE_SCORES.safe_knight;
    }

    // Mobility and space control
    if (Component == SC_MOBILITY || Component == SC_ALL) {
      // Center control bonus
      score += PIECE_SCORES.control_center_knight *
               Value(count_bits(attacking & center_bits));

      // Knight outpost
      BITBOARD outpost = pawn_attacks_bb(Them, s) & pos.get_pieces_bb(PAWN, Us);

      // If outpost exists and there is no pawn attacking the knight
      // square
      if (outpost && !(pawn_attacks_bb(Us, s) & pos.get_pieces_bb(PAWN, Them)))
        score += PIECE_SCORES.outpost_knight * Value(count_bits(outpost));

      // Control space bonus
      score += PIECE_SCORES.control_space[KNIGHT] *
               Value(count_bits(attacking & opp_ranks));

      // Mobility bonus
      BITBOARD moves = get_real_possible_moves(Us, pos, s, attacking);
      score += PIECE_SCORES.mobility[KNIGHT] * Value(count_bits(moves));
    }

    if (Component == SC_KING_SAFETY || Component == SC_ALL) {
      // Knights and bishopos should not be protecting the king in the
      // middlegame
      score += KING_SAFETY_SCORES.protector_penalty[KNIGHT] *
               Value(distance(pos.square<KING>(Us), s));

      // Check for opponent pieces attacking the knight square
      score += KING_SAFETY_SCORES.attacker_penalty[KNIGHT] *
               Value(distance(pos.square<KING>(Them), s));
    }
  }

  return score;
}

template <ScoreComponent Component, Color Us>
Score score_bishops(const Position &pos) {
  constexpr Color Them = ~Us;
  constexpr BITBOARD opp_ranks = opponent_ranks_for(Us);

  Score score = 0;

  BITBOARD bishops = pos.get_pieces_bb(BISHOP, Us);

  if (Component == SC_MATERIAL || Component == SC_ALL)
    score += MATERIAL_SCORES.piece_value[BISHOP] * Value(pos.count<BISHOP>(Us));

  if (Component == SC_MATERIAL) return score;

  if ((Component == SC_PIECE_COORDINATION || Component == SC_ALL) &&
      pos.count<BISHOP>(Us) >= 2)
    score += PIECE_SCORES.bishop_pair;

  while (bishops) {
    Square s = pop_ls1b(bishops);
    BITBOARD attacking = attacks_bb_by<BISHOP>(s, pos.get_all_pieces_bb());
    BITBOARD blockers =
        pos.get_all_pieces_bb() &
        ~(pos.get_pieces_bb(BISHOP, Us) & pos.get_pieces_bb(QUEEN, Us));

    // Mobility checks
    if (Component == SC_MOBILITY || Component == SC_ALL) {
      BITBOARD control = attacks_bb_by<BISHOP>(s, blockers);
      score += PIECE_SCORES.control_space[BISHOP] *
               Value(count_bits(control & opp_ranks));

      BITBOARD moves = get_real_possible_moves(Us, pos, s, attacking);
      score += PIECE_SCORES.mobility[BISHOP] * Value(count_bits(moves));

      // Get the square color of the bishop
      BITBOARD color_complex =
          (square_color(s) == WHITE) ? LightSquares : DarkSquares;

      // Intersect our pawns with the color complex of the bishop
      // This represents blocking the bishop's view
      // Though in some cases pawns can be considered as an extension of
      // the bishop
      score += PAWN_STRUCTURE_SCORES.same_color_as_bishop *
               Value(count_bits(pos.get_pieces_bb(PAWN, Us) & color_complex));
    }

    if (Component == SC_KING_SAFETY || Component == SC_ALL) {
      Square ours = pos.square<KING>(Us);
      Square theirs = pos.square<KING>(Them);

      // Bishops and knights should not be protecting the king
      // If they do, at least it should not be from a mile away
      score += KING_SAFETY_SCORES.protector_penalty[BISHOP] *
               Value(square_distance[ours][s]);
      score += KING_SAFETY_SCORES.attacker_penalty[BISHOP] *
               Value(square_distance[theirs][s]);
    }

    if (Component == SC_PIECE_COORDINATION || Component == SC_ALL) {
      // Bishop outpost
      BITBOARD outpost = pawn_attacks_bb(Them, s) & pos.get_pieces_bb(PAWN, Us);

      // If outpost exists and there is no pawn attacking the bishop
      // square
      if (outpost && !(pawn_attacks_bb(Us, s) & pos.get_pieces_bb(PAWN, Them)))
        score += PIECE_SCORES.outpost_bishop * Value(count_bits(outpost));
    }
  }

  return score;
}

template <ScoreComponent Component, Color Us>
Score score_rooks(const Position &pos) {
  constexpr Color Them = ~Us;
  Score score = 0;

  BITBOARD rooks = pos.get_pieces_bb(ROOK, Us);
  BITBOARD opp_ranks = opponent_ranks_for(Us);

  if (Component == SC_MATERIAL || Component == SC_ALL)
    score += MATERIAL_SCORES.piece_value[ROOK] * Value(pos.count<ROOK>(Us));

  if (Component == SC_MATERIAL) return score;

  while (rooks) {
    Square s = pop_ls1b(rooks);
    BITBOARD file = file_bb(s);

    BITBOARD attacking = attacks_bb_by<ROOK>(s, pos.get_all_pieces_bb());
    BITBOARD blockers =
        pos.get_all_pieces_bb() &
        ~(pos.get_pieces_bb(ROOK, Us) & pos.get_pieces_bb(QUEEN, Us));

    if (Component == SC_MOBILITY || Component == SC_ALL) {
      BITBOARD control_bb =
          attacks_bb_by<ROOK>(s, blockers) & pos.get_all_empty_squares_bb();
      score += PIECE_SCORES.control_space[ROOK] *
               Value(count_bits(control_bb & opp_ranks));

      BITBOARD moves = get_real_possible_moves(Us, pos, s, attacking);
      score += PIECE_SCORES.mobility[ROOK] * Value(count_bits(moves));

      // Open/semi-open files bonuses
      if (!(file & pos.get_pieces_bb(PAWN))) {
        score += PIECE_SCORES.rook_open_file;
      }

      if (!(file & pos.get_pieces_bb(PAWN, Us)) &&
          !(file & pos.get_pieces_bb(PAWN, Them))) {
        score += PIECE_SCORES.rook_semi_open_file;
      }

      // Mobility check
      // - Identifies rooks with <= 3 moves
      if (count_bits(moves) <= 3) {
        Square ksq = pos.square<KING>(Us);
        File kings_file = file_of(ksq);

        // Checks if the rook is on the same side as the king
        if ((kings_file < FILE_E) == (file_of(s) < kings_file)) {
          // Checks for castling availability of the side
          // [us & ANY] will return:
          // - WK and/or WQ for white
          // - BK and/or BQ for black
          const bool can_castle = pos.can_castle(Us & ANY);
          score += PIECE_SCORES.trapped_rook * Value(1 + Value(!can_castle));
        }
      }
    }
    if (Component == SC_PIECE_COORDINATION || Component == SC_ALL) {
      // Check if the other rook is present on the board
      // This is possible because we only popped one rook from the
      // bitboard
      Square other = get_ls1b(rooks);

      bool same_file = (other != NONE) ? (file_of(s) == file_of(other)) : false;
      bool same_rank = (other != NONE) ? (rank_of(s) == rank_of(other)) : false;

      if (same_file || same_rank) {
        score += (PIECE_SCORES.connected_rooks / 2);
      }
    }
  }

  return score;
}

template <ScoreComponent Component, Color Us>
Score score_queens(const Position &pos) {
  constexpr Color Them = ~Us;
  Score score = 0;

  BITBOARD queens = pos.get_pieces_bb(QUEEN, Us);
  BITBOARD opp_ranks = opponent_ranks_for(Us);

  if (Component == SC_MATERIAL || Component == SC_ALL)
    score += MATERIAL_SCORES.piece_value[QUEEN] * Value(pos.count<QUEEN>(Us));

  if (Component == SC_MATERIAL) return score;

  while (queens) {
    Square s = pop_ls1b(queens);

    if (Component == SC_MOBILITY || Component == SC_ALL) {
      // Bishop-like control
      BITBOARD blockers =
          pos.get_all_pieces_bb() &
          ~(pos.get_pieces_bb(QUEEN, Us) | pos.get_pieces_bb(BISHOP, Us));
      BITBOARD control = attacks_bb_by<BISHOP>(s, blockers);

      score += PIECE_SCORES.control_space[QUEEN] *
               Value(count_bits(control & opp_ranks));

      // Rook-like control
      blockers = pos.get_all_pieces_bb() &
                 ~(pos.get_pieces_bb(QUEEN, Us) | pos.get_pieces_bb(ROOK, Us));
      control = attacks_bb_by<ROOK>(s, blockers);

      score += PIECE_SCORES.control_space[QUEEN] *
               Value(count_bits(control & opp_ranks));

      // Mobility bonus
      BITBOARD attacking = attacks_bb_by<QUEEN>(s, pos.get_all_pieces_bb());
      BITBOARD moves = get_real_possible_moves(Us, pos, s, attacking);
      score += PIECE_SCORES.mobility[QUEEN] * Value(count_bits(moves));
    }

    // Queen safety/vulnerability scoring
    if (Component == SC_PIECE_COORDINATION || Component == SC_ALL) {
      // From queens square fire up an attack
      // Then intersect it with the opponent snipers that are with less
      // value than the queen
      BITBOARD snipers =
          (attacks_bb_by<BISHOP>(s) & pos.get_pieces_bb(BISHOP, Them)) |
          (attacks_bb_by<ROOK>(s) & pos.get_pieces_bb(ROOK, Them));

      // All pieces without the snipers and our queen
      BITBOARD rest = pos.get_all_pieces_bb() ^ (snipers | s);

      while (snipers) {
        Square sniper_sq = pop_ls1b(snipers);
        BITBOARD btw = in_between_bb(s, sniper_sq) & rest;

        // if there is space between the queen square and the slider
        // and is still left space after removing a bit
        // then there is greater than or equal to one blocker between
        // them
        if (btw && !has_bit_after_pop(btw)) {
          score += PIECE_SCORES.vulnerable_queen;
          break;
        }
      }
    }
  }

  return score;
}

template <Color Us>
Score score_king_shelter(const Position &pos, Square ksq) {
  BITBOARD pawns_in_area =
      attacks_bb_by<KING>(ksq) & pos.get_pieces_bb(PAWN, Us);
  return KING_SAFETY_SCORES.bonus * Value(count_bits(pawns_in_area));
}

template <Color Us>
Score score_king_safety(const Position &pos) {
  const Square ksq = pos.square<KING>(Us);
  const CastlingRights kingside = Us & KINGSIDE;
  const CastlingRights queenside = Us & QUEENSIDE;

  // Will return WK/BK for white/black
  // Will return WQ/BQ for white/black

  Score score = 0;

  auto compare_scores = [](Score a, Score b) { return a.mg < b.mg; };

  // If we can castle kingside check if there is a better shelter
  if (pos.can_castle(kingside)) {
    score += std::max(score_king_shelter<Us>(pos, ksq),
                      score_king_shelter<Us>(pos, sq_relative_to_side(G1, Us)),
                      compare_scores);
  }

  // If we can castle queen side check if there is a better shelter
  if (pos.can_castle(queenside)) {
    score += std::max(score,
                      score_king_shelter<Us>(pos, sq_relative_to_side(C1, Us)),
                      compare_scores);

    score += std::max(score,
                      score_king_shelter<Us>(pos, sq_relative_to_side(B1, Us)),
                      compare_scores);
  }

  // Apply penalty for distance from pawns in endgame
  // Tries to bring king closer to the pawns
  BITBOARD pawns = pos.get_pieces_bb(PAWN, Us);

  while (pawns)
    score += KING_SAFETY_SCORES.pawn_proximity_penalty *
             Value(distance(pop_ls1b(pawns), ksq));

  return score;
}

template <ScoreComponent Component, Color Us>
Score score_king(const Position &pos) {
  const Color Them = ~Us;
  const Square our_ksq = pos.square<KING>(Us);
  const Square enemy_ksq = pos.square<KING>(Them);

  Score score = 0;

  // Account for mobility
  if (Component == SC_MOBILITY || Component == SC_ALL) {
    BITBOARD moves =
        attacks_bb_by<KING>(our_ksq) & ~pos.get_attacked_squares(Them);
    score += PIECE_SCORES.mobility[KING] * Value(count_bits(moves));
  }

  // King safety
  if (Component == SC_KING_SAFETY || Component == SC_ALL) {
    score += score_king_safety<Us>(pos);

    // Checking for backrank weakness
    const Rank first_rank = rank_relative_to_side(Us, RANK_1);
    const Rank second_rank = rank_relative_to_side(Us, RANK_2);
    BITBOARD king_area = attacks_bb_by<KING>(our_ksq) | our_ksq;
    BITBOARD their_rook_and_queen =
        pos.get_pieces_bb(ROOK, QUEEN) & pos.get_pieces_bb(Them);

    if (rank_of(our_ksq) == first_rank && their_rook_and_queen) {
      // Check for backrank weakness
      BITBOARD back_rank_area = king_area & rank_bb(second_rank);

      // Get our pieces and combine them with the minor pieces attacks
      // This is then combined with opponent's pawn attacks and king
      // attacks from the enemy king Finally we get a bitboard containing
      // our pieces, opposite king attacks, minor pieces attacks and
      // opponent's pawn attacks. Meaning squares that are not available
      // for our king to move to
      BITBOARD blocked =
          pos.get_pieces_bb(Us) |
          (pos.get_attacks_by<KNIGHT>(Us) | pos.get_attacks_by<BISHOP>(Us) |
           pos.get_attacks_by<ROOK>(Us) | pos.get_attacks_by<QUEEN>(Us)) |
          pos.get_attacks_by<PAWN>(Them) | attacks_bb_by<KING>(enemy_ksq);

      if ((back_rank_area & blocked) == back_rank_area)
        score += KING_SAFETY_SCORES.weak_back_rank;
    }

    // This approach will work becase as we evaluate for the both sides
    // Penalties will cancel out at some point if position is equal

    // We'll do this in two steps

    // Step 1: Detect weak diagonals

    // If opponent has a queen or a bishop is on the same color as our king
    // Then apply weak diagonal penalty times the possible squares where a
    // bishop could deliver check

    // This will generate all the bishop squares that might cause check
    // without the king blockers
    BITBOARD snipers = attacks_bb_by<BISHOP>(
        our_ksq, pos.get_all_pieces_bb() ^ pos.get_king_blockers(Us));

    // Get the square color of our king
    BITBOARD color_complex =
        (square_color(our_ksq) == WHITE) ? LightSquares : DarkSquares;

    // If opponent has a queen (which can change color complexes easily)
    // or a bishop that is the same color as the king's square (the same
    // color complex. Making it a weak diagonal)
    if (pos.get_pieces_bb(QUEEN, Them) ||
        (pos.get_pieces_bb(BISHOP, Them) & color_complex))
      score += KING_SAFETY_SCORES.weak_diagonals * Value(count_bits(snipers));

    // Step 2: Detect weak files and ranks

    // If opponent has a queen or rook
    // Apply weak king lines penalty times the possible squares where a rook
    // could deliver check

    // This will get the attacked squares by the rook without the king
    // blockers meaning each bit represents a square when a piece lands
    // there it should give a check
    snipers = attacks_bb_by<ROOK>(
        our_ksq, pos.get_all_pieces_bb() ^ pos.get_king_blockers(Us));

    if (pos.get_pieces_bb(QUEEN, Them) || pos.get_pieces_bb(ROOK, Them))
      score += KING_SAFETY_SCORES.weak_lines * Value(count_bits(snipers));
  }

  return score;
}

// Generates all the material needed
template <ScoreComponent Component, Color Us>
Score score_all_material(const Position &pos) {
  Score score = 0;

  score += score_pawns<Component, Us>(pos);
  score += score_knights<Component, Us>(pos);
  score += score_bishops<Component, Us>(pos);
  score += score_rooks<Component, Us>(pos);
  score += score_queens<Component, Us>(pos);
  score += score_king<Component, Us>(pos);

  return score;
}

std::string component_type(ScoreComponent c) {
  switch (c) {
    case SC_MATERIAL:
      return "Material";
    case SC_MOBILITY:
      return "Mobility";
    case SC_KING_SAFETY:
      return "King Safety";
    case SC_PAWN_STRUCTURE:
      return "Pawn Structure";
    case SC_PIECE_COORDINATION:
      return "Piece Coordination";
    case SC_ALL:
      return "All";
  }

  return "";
}
}  // namespace

// SC_MATERIAL 				- Gets the material score for the given
// side SC_MOBILITY 				- Gets the mobility score for
// the given side SC_KING_SAFETY 			- Gets the king safety
// score for the given side SC_PAWN_STRUCTURE 		- Gets the pawn
// structure score for the given side SC_PIECE_COORDINATION	- Gets the piece
// coordination score for the given side SC_ALL
// - Gets the total score for the given side

template <ScoreComponent Component>
Score total_scores(const Position &pos) {
  return score_all_material<Component, WHITE>(pos) -
         score_all_material<Component, BLACK>(pos);
}

template <ScoreComponent T>
inline Value Scorer<T>::get_score(const Position &pos) {
  // Get the endgame scores
  Value endgame = Endgames::score(pos);

  if (endgame != VALUE_NONE) return endgame;

  // Get the game phase weights
  weight = game_phase_weights(pos);

  // Get the score for the given component
  score = total_scores<T>(pos);

  // Combine the score with the weight
  Value v = combine(score, weight);

  return pos.side_to_move() == WHITE ? v : -v;
}

template <ScoreComponent T>
inline void Scorer<T>::print_stats(const Position &pos) {
#define P(white, black) white << "|" << black << "|" << (white - black) << "|"

  std::cout << pos << std::endl;

  Score w_score = score_all_material<T, WHITE>(pos);
  Score b_score = score_all_material<T, BLACK>(pos);

  std::cout
      << std::showpoint << std::noshowpos << std::fixed << std::setprecision(2)
      << "+---------+--------------+--------------+--------------+" << std::endl
      << "| Scoring by: " << component_type(T) << std::endl
      << "| Total score: " << get_score(pos) << std::endl
      << "+---------+--------------+--------------+--------------+" << std::endl
      << "|  TYPE   |     White    |     Black    |     Total    |" << std::endl
      << "|         |   Mg    Eg   |   Mg    Eg   |   Mg    Eg   |" << std::endl
      << "+---------+--------------+--------------+--------------+" << std::endl
      << "|   PAWNS |"
      << P((score_pawns<T, WHITE>(pos)), (score_pawns<T, BLACK>(pos)))
      << std::endl
      << "| KNIGHTS |"
      << P((score_knights<T, WHITE>(pos)), (score_knights<T, BLACK>(pos)))
      << std::endl
      << "| BISHOPS |"
      << P((score_bishops<T, WHITE>(pos)), (score_bishops<T, BLACK>(pos)))
      << std::endl
      << "|   ROOKS |"
      << P((score_rooks<T, WHITE>(pos)), (score_rooks<T, BLACK>(pos)))
      << std::endl
      << "|  QUEENS |"
      << P((score_queens<T, WHITE>(pos)), (score_queens<T, BLACK>(pos)))
      << std::endl
      << "|    KING |"
      << P((score_king<T, WHITE>(pos)), (score_king<T, BLACK>(pos)))
      << std::endl
      << "+---------+--------------+--------------+--------------+" << std::endl
      << "|   TOTAL |" << P(w_score, b_score) << std::endl
      << "|  WEIGHT |" << Score(weight, MAX_PIECE_WEIGHTS)
      << "|              |              |" << std::endl
      << "+---------+--------------+--------------+--------------+"
      << std::endl;
#undef P
}

template Score total_scores<SC_MATERIAL>(const Position &pos);
template Score total_scores<SC_MOBILITY>(const Position &pos);
template Score total_scores<SC_KING_SAFETY>(const Position &pos);
template Score total_scores<SC_PAWN_STRUCTURE>(const Position &pos);
template Score total_scores<SC_PIECE_COORDINATION>(const Position &pos);
template Score total_scores<SC_ALL>(const Position &pos);

template void Scorer<SC_MATERIAL>::print_stats(const Position &);
template void Scorer<SC_MOBILITY>::print_stats(const Position &);
template void Scorer<SC_KING_SAFETY>::print_stats(const Position &);
template void Scorer<SC_PAWN_STRUCTURE>::print_stats(const Position &);
template void Scorer<SC_PIECE_COORDINATION>::print_stats(const Position &);
template void Scorer<SC_ALL>::print_stats(const Position &);

template Value Scorer<SC_MATERIAL>::get_score(const Position &);
template Value Scorer<SC_MOBILITY>::get_score(const Position &);
template Value Scorer<SC_KING_SAFETY>::get_score(const Position &);
template Value Scorer<SC_PAWN_STRUCTURE>::get_score(const Position &);
template Value Scorer<SC_PIECE_COORDINATION>::get_score(const Position &);
template Value Scorer<SC_ALL>::get_score(const Position &);

/*
inline std::int32_t score_move(std::int32_t move)
{
        // score capture moves
        if (get_move_capture(move))
        {
                // target piece
                Piece target = WHITE_PAWN;

                Piece start = !side ? BLACK_PAWN : WHITE_PAWN;
                Piece end = !side ? BLACK_KING : WHITE_KING;

                for (Piece p = start; p <= end; ++p)
                {
                        if (get_bit(bitboards[p], get_move_target(move)))
                        {
                                target = p;
                                break;
                        }
                }

                // score move by MVV LVA lookup [source][target]
                return MVV_LVA[get_move_piece(move)][target];
        }
        else // quiet move
        {

        }

        return 0;
}

std::int32_t sort_move(moves* move_list)
{
        // move scores
        std::unique_ptr<std::int32_t[]> move_scores =
std::make_unique<std::int32_t[]>(move_list->count);

        std::cout << "\n\n";
        for (std::int32_t c = 0; c < move_list->count; c++)
                move_scores[c] = score_move(move_list->moves[c]);

        for (std::int32_t curr_mv = 0; curr_mv < move_list->count; curr_mv++)
                for (std::int32_t next_mv = curr_mv + 1; next_mv <
move_list->count; next_mv++)
                {
                        // cmp current and next move scores
                        if (move_scores[curr_mv] < move_scores[next_mv])
                        {
                                // swap the scores
                                std::swap(move_scores[curr_mv],
move_scores[next_mv]);
                                // swap the moves
                                std::swap(move_list->moves[curr_mv],
move_list->moves[next_mv]);
                        }
                }

        return 0;
}

void print_move_scores(moves* move_list)
{
        printf("     Move scores:\n\n");

        // loop over moves within a move list
        for (std::int32_t count = 0; count < move_list->count; count++)
        {
                printf("     move: ");
                print_move(move_list->moves[count]);
                printf(" score: %d\n", score_move(move_list->moves[count]));
        }
}
*/
}  // namespace KhaosChess
