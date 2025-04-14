#include "score.h"

#include "defs.h"
#include "move.h"
#include "consts.h"
#include "position.h"
#include "bitboard.h"

#include <algorithm>
#include <memory>

namespace KhaosChess
{
	namespace
	{
		BITBOARD get_real_possible_moves(const Color c, const Position &pos, Square s, BITBOARD moves)
		{
			const Square ksq = pos.square<KING>(c);

			// Get all opponent pieces, except pawns
			BITBOARD opp_pieces = pos.get_pieces_bb(~c) ^ pos.get_pieces_bb(PAWN, ~c);

			if (pos.get_king_blockers(c) & ksq)
				moves &= in_between_bb(s, ksq);

			// cannot move into square with our piece
			moves &= ~pos.get_pieces_bb(c);

			// Get the other sides's pawn attacks into one bitboard
			BITBOARD pawns = pos.get_pieces_bb(PAWN, ~c);
			BITBOARD pawn_attacks = 0ULL;

			while (pawns)
				pawn_attacks |= pawn_attacks_bb(~c, pop_ls1b(pawns));

			// Don't count squares attacked by opponents pawns with nothing valuable there
			moves &= ~(pawn_attacks & ~pos.get_pieces_bb(~c));

			return moves;
		}

		// Game phase calculation based on piece material
		int calculate_game_phase(const Position &pos)
		{
			int phase = 0;

			// Count total material for both sides (excluding pawns and kings)
			for (Color c : {WHITE, BLACK})
				phase += (pos.count<KNIGHT>(c) + pos.count<BISHOP>(c) +
						  2 * pos.count<ROOK>(c) +
						  4 * pos.count<QUEEN>(c));

			// Scale phase to 0-24 range (0 = opening, 24 = endgame)
			// MaxPhase = 16 (all pieces on board)
			const int MaxPhase = 16;
			return (phase * MAX_PHASE_SCORE) / MaxPhase;
		}

		template <ScoreComponent Component, Color Us>
		Score score_pawns(const Position &pos, Score &score)
		{
			constexpr Color Them = ~Us;
			constexpr Direction Up = pawn_push_direction(Us);
			constexpr Direction Down = pawn_push_direction(Them);
			constexpr int inc = Us == WHITE ? 1 : -1;

			BITBOARD our_pawns = pos.get_pieces_bb(PAWN, Us);
			BITBOARD opp_pawns = pos.get_pieces_bb(PAWN, Them);

			// Return early if we only need material evaluation
			if (Component == SC_MATERIAL || Component == SC_ALL)
				score += MATERIAL_SCORES.piece_value[PAWN] * Value(pos.count<PAWN>(Us));

			// Return early if we only need material evaluation
			if (Component == SC_MATERIAL)
				return score;

			// Center controlled squares by side
			BITBOARD center_squares = FileC_Bits | FileD_Bits | FileE_Bits | FileF_Bits;
			center_squares &= (Us == WHITE) ? Rank5_Bits | Rank6_Bits : Rank3_Bits | Rank4_Bits;

			BITBOARD pawns = our_pawns;
			while (pawns)
			{
				// Get the pawn structure components
				if (Component == SC_PAWN_STRUCTURE || Component == SC_ALL)
				{
					Square s = pop_ls1b(pawns);

					Rank r = rank_of(s);
					Rank rel_r = rank_relative_to_side(Us, r);
					File f = file_of(s);
					Rank promotion_rank = Us == WHITE ? RANK_8 : RANK_1;

					const BITBOARD attacks = pawn_attacks_bb(Us, s);			   // our attacks
					const BITBOARD neighbours = our_pawns & neighbouring_files[f]; // Gets our pawns located on the adjacent files
					const BITBOARD phalanx = neighbours & rank_bb(r);			   // Pawns that located side by side on adjacent files
					const BITBOARD support = neighbours & rank_bb(Rank(r - inc));  // Checks the adjacent rank for supporting neighbouring pawns
					const BITBOARD lever = opp_pawns & attacks;					   // Checks for opponent pawns that can be captured
					const BITBOARD lever_push = opp_pawns & move_to<Up>(attacks);  // Checks for opponent pawns that can be pushed
					const BITBOARD opposed = opp_pawns & passed_pawn_path<Us>(s);  // Checks for opponent pawns that are opposed to our pawns

					const bool are_blocked = opp_pawns & move_to<Up>(square_to_BB(s));
					const bool are_doubled = our_pawns & move_to<Down>(square_to_BB(s));
					const bool is_backward = !(neighbours & passed_pawn_path<Them>(s + Up)) && (are_blocked || lever_push);
					const bool is_passed = !opposed ||
										   !(opposed ^ lever) ||
										   (!(opposed ^ lever_push) &&
											count_bits(phalanx) >= count_bits(lever_push));

					score += count_bits(attacks & center_squares) * PAWN_STRUCTURE_SCORES.control_center;

					if (are_doubled)
						score += PAWN_STRUCTURE_SCORES.doubled;
					if (support | phalanx)
					{
						score += Score(PAWN_STRUCTURE_SCORES.connected_bonus[rel_r] *
									   (1 + static_cast<int>(bool(phalanx)) - static_cast<int>(bool(opposed))));
						score += Score(10 * count_bits(support));
					}
					else if (!neighbours)
						score += PAWN_STRUCTURE_SCORES.isolated;
					else if (is_backward)
						score += PAWN_STRUCTURE_SCORES.backward;

					if (is_passed)
						score += PAWN_STRUCTURE_SCORES.passed * PAWN_STRUCTURE_SCORES.passed_rank_weight[rel_r];
				}

				// King safety components (pawn shield)
				if (Component == SC_KING_SAFETY || Component == SC_ALL)
				{
					Square ksq = pos.square<KING>(Us);

					// Check for king shelter
					BITBOARD pawns_in_kings_area = attacks_bb_by<KING>(ksq) & our_pawns;
					score += Value(count_bits(pawns_in_kings_area)) * KING_SAFETY_SCORES.bonus;
				}
			}

			return score;
		}

		template <ScoreComponent Component, Color Us>
		Score score_knights(const Position &pos, Score &score)
		{
			constexpr Color Them = ~Us;
			constexpr BITBOARD center_bits = FileC_Bits | FileD_Bits | FileE_Bits | FileF_Bits;
			constexpr BITBOARD opp_ranks = Us == WHITE
											   ? Rank8_Bits | Rank7_Bits | Rank6_Bits | Rank5_Bits
											   : Rank1_Bits | Rank2_Bits | Rank3_Bits | Rank4_Bits;

			BITBOARD knights = pos.get_pieces_bb(KNIGHT, Us);

			if (Component == SC_MATERIAL || Component == SC_ALL)
				score += MATERIAL_SCORES.piece_value[KNIGHT] * Value(pos.count<KNIGHT>(Us));

			if (Component == SC_MATERIAL)
				return score;

			while (knights)
			{
				Square s = pop_ls1b(knights);
				BITBOARD attacking = attacks_bb_by<KNIGHT>(s);

				// Piece coordination
				if (Component == SC_PIECE_COORDINATION || Component == SC_ALL)
				{
					if (pos.get_attacks_by<PAWN>(Us) & square_to_BB(s))
						score += PIECE_SCORES.safe_knight;
				}

				// Mobility and space control
				if (Component == SC_MOBILITY || Component == SC_ALL)
				{
					// Center control bonus
					score += PIECE_SCORES.control_center_knight * Value(count_bits(attacking & center_bits));

					// Knight outpost
					BITBOARD outpost = pawn_attacks_bb(Them, s) & pos.get_pieces_bb(PAWN, Us);

					// If outpost exists and there is no pawn attacking the knight square
					if (outpost && !(pawn_attacks_bb(Us, s) & pos.get_pieces_bb(PAWN, Them)))
						score += PIECE_SCORES.outpost_knight * Value(count_bits(outpost));

					// Control space bonus
					score += PIECE_SCORES.control_space[KNIGHT] * Value(count_bits(attacking & opp_ranks));

					// Mobility bonus
					BITBOARD moves = get_real_possible_moves(Us, pos, s, attacking);
					score += PIECE_SCORES.mobility[KNIGHT] * Value(count_bits(moves));
				}

				if (Component == SC_KING_SAFETY || Component == SC_ALL)
				{
					// Knights and bishopos should not be protecting the king in the middlegame
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
		Score score_bishops(const Position &pos, Score &score)
		{
			constexpr Color Them = ~Us;
			constexpr BITBOARD center_bits = FileC_Bits | FileD_Bits | FileE_Bits | FileF_Bits;
			constexpr BITBOARD opp_ranks = Us == WHITE
											   ? Rank8_Bits | Rank7_Bits | Rank6_Bits | Rank5_Bits
											   : Rank1_Bits | Rank2_Bits | Rank3_Bits | Rank4_Bits;

			BITBOARD bishops = pos.get_pieces_bb(BISHOP, Us);

			if (Component == SC_MATERIAL || Component == SC_ALL)
				score += MATERIAL_SCORES.piece_value[BISHOP] * Value(pos.count<BISHOP>(Us));

			if (Component == SC_MATERIAL)
				return score;

			if ((Component == SC_PIECE_COORDINATION || Component == SC_ALL) && pos.count<BISHOP>(Us) >= 2)
				score += PIECE_SCORES.bishop_pair;

			while (bishops)
			{
				Square s = pop_ls1b(bishops);
				BITBOARD attacking = attacks_bb_by<BISHOP>(s, pos.get_all_pieces_bb());
				BITBOARD blockers = pos.get_all_pieces_bb() & ~(pos.get_pieces_bb(BISHOP, Us) & pos.get_pieces_bb(QUEEN, Us));

				// Mobility checks
				if (Component == SC_MOBILITY || Component == SC_ALL)
				{
					BITBOARD control = attacks_bb_by<BISHOP>(s, blockers);
					score += PIECE_SCORES.control_space[BISHOP] * Value(count_bits(control & opp_ranks));

					BITBOARD moves = get_real_possible_moves(Us, pos, s, attacking);
					score += PIECE_SCORES.mobility[BISHOP] * Value(count_bits(moves));

					// Get the square color of the bishop
					Color sq_color = (int(rank_of(s)) + int(file_of(s))) & 1 ? WHITE : BLACK;
					BITBOARD color_complex = (sq_color == WHITE) ? LightSquares : DarkSquares;

					// Intersect our pawns with the color complex of the bishop
					// This represents blocking the bishop's view
					// Though in some cases pawns can be considered as an extension of the bishop
					score += PAWN_STRUCTURE_SCORES.same_color_as_bishop *
							 Value(count_bits(pos.get_pieces_bb(PAWN, Us) & color_complex));
				}

				if (Component == SC_KING_SAFETY || Component == SC_ALL)
				{
					Square ours = pos.square<KING>(Us);
					Square theirs = pos.square<KING>(Them);

					// Bishops and knights should not be protecting the king
					// If they do, at least it should not be from a mile away
					score += KING_SAFETY_SCORES.protector_penalty[BISHOP] * Value(square_distance[ours][s]);
					score += KING_SAFETY_SCORES.attacker_penalty[BISHOP] * Value(square_distance[theirs][s]);
				}

				if (Component == SC_PIECE_COORDINATION || Component == SC_ALL)
				{
					// Bishop outpost
					BITBOARD outpost = pawn_attacks_bb(Them, s) & pos.get_pieces_bb(PAWN, Us);

					// If outpost exists and there is no pawn attacking the bishop square
					if (outpost && !(pawn_attacks_bb(Us, s) & pos.get_pieces_bb(PAWN, Them)))
						score += PIECE_SCORES.outpost_bishop * Value(count_bits(outpost));
				}
			}

			return score;
		}

		template <ScoreComponent Component, Color Us>
		Score score_rooks(const Position &pos, Score &score)
		{
			constexpr Color Them = ~Us;
			BITBOARD rooks = pos.get_pieces_bb(ROOK, Us);
			BITBOARD opp_ranks = Us == WHITE
									 ? Rank8_Bits | Rank7_Bits | Rank6_Bits | Rank5_Bits
									 : Rank1_Bits | Rank2_Bits | Rank3_Bits | Rank4_Bits;

			if (Component == SC_MATERIAL || Component == SC_ALL)
				score += MATERIAL_SCORES.piece_value[ROOK] * Value(pos.count<ROOK>(Us));

			if (Component == SC_MATERIAL)
				return score;

			while (rooks)
			{
				Square s = pop_ls1b(rooks);
				BITBOARD file = file_bb(s);
				BITBOARD rank = rank_bb(s);

				BITBOARD attacking = attacks_bb_by<ROOK>(s, pos.get_all_pieces_bb());
				BITBOARD blockers = pos.get_all_pieces_bb() &
									~(pos.get_pieces_bb(ROOK, Us) & pos.get_pieces_bb(QUEEN, Us));

				if (Component == SC_MOBILITY || Component == SC_ALL)
				{
					BITBOARD control_bb = attacks_bb_by<ROOK>(s, blockers) & pos.get_all_empty_squares_bb();
					score += PIECE_SCORES.control_space[ROOK] * Value(count_bits(control_bb & opp_ranks));

					BITBOARD moves = get_real_possible_moves(Us, pos, s, attacking);
					score += PIECE_SCORES.mobility[ROOK] * Value(count_bits(moves));

					// Open/semi-open files bonuses
					if (!(file & pos.get_pieces_bb(PAWN)))
						score += PIECE_SCORES.rook_open_file;
					else if (!(file & pos.get_pieces_bb(PAWN, Us)) && !(file & pos.get_pieces_bb(PAWN, ~Us)))
						score += PIECE_SCORES.rook_semi_open_file;

					// Mobility check
					// - Identifies rooks with <= 3 moves
					if (count_bits(moves) <= 3)
					{
						Square ksq = pos.square<KING>(Us);
						File kings_file = file_of(ksq);

						// Checks if the rook is on the same side as the king
						if ((kings_file < FILE_E) == (file_of(s) < kings_file))
						{
							// Checks for castling availability of the side
							// [us & ANY] will return:
							// - WK and/or WQ for white
							// - BK and/or BQ for black
							const bool can_castle = pos.can_castle(Us & ANY);
							score += PIECE_SCORES.trapped_rook * Value(1 + Value(!can_castle));
						}
					}
				}
				if (Component == SC_PIECE_COORDINATION || Component == SC_ALL)
				{
					// Check if the other rook is present on the board
					// This is possible because we only popped one rook from the bitboard
					Square other = get_ls1b(rooks);

					bool same_file = other != NONE ? file_of(s) == file_of(other) : false;
					bool same_rank = other != NONE ? rank_of(s) == rank_of(other) : false;

					if (same_file || same_rank)
						score += PIECE_SCORES.connected_rooks / 2;
				}
			}

			return score;
		}

		template <ScoreComponent Component, Color Us>
		Score score_queens(const Position &pos, Score &score)
		{
			constexpr Color Them = ~Us;
			BITBOARD queens = pos.get_pieces_bb(QUEEN, Us);
			BITBOARD opp_ranks = Us == WHITE
									 ? Rank8_Bits | Rank7_Bits | Rank6_Bits | Rank5_Bits
									 : Rank1_Bits | Rank2_Bits | Rank3_Bits | Rank4_Bits;

			if (Component == SC_MATERIAL || Component == SC_ALL)
				score += MATERIAL_SCORES.piece_value[QUEEN] * Value(pos.count<QUEEN>(Us));

			if (Component == SC_MATERIAL)
				return score;

			while (queens)
			{
				Square s = pop_ls1b(queens);

				if (Component == SC_MOBILITY || Component == SC_ALL)
				{
					// Bishop-like control
					BITBOARD blockers = pos.get_all_pieces_bb() & ~(pos.get_pieces_bb(QUEEN, Us) | pos.get_pieces_bb(BISHOP, Us));
					BITBOARD control = attacks_bb_by<BISHOP>(s, blockers);

					score += PIECE_SCORES.control_space[QUEEN] * Value(count_bits(control & opp_ranks));

					// Rook-like control
					blockers = pos.get_all_pieces_bb() & ~(pos.get_pieces_bb(QUEEN, Us) | pos.get_pieces_bb(ROOK, Us));
					control = attacks_bb_by<ROOK>(s, blockers);

					score += PIECE_SCORES.control_space[QUEEN] * Value(count_bits(control & opp_ranks));

					// Mobility bonus
					BITBOARD attacking = attacks_bb_by<QUEEN>(s, pos.get_all_pieces_bb());
					BITBOARD moves = get_real_possible_moves(Us, pos, s, attacking);
					score += PIECE_SCORES.mobility[QUEEN] * Value(count_bits(moves));
				}

				// Queen safety/vulnerability scoring
				if (Component == SC_PIECE_COORDINATION || Component == SC_ALL)
				{
					// From queens square fire up an attack
					// Then intersect it with the oppnent snipers that are with less value than the queen
					BITBOARD snipers = (attacks_bb_by<BISHOP>(s) & pos.get_pieces_bb(BISHOP, Them)) |
									   (attacks_bb_by<ROOK>(s) & pos.get_pieces_bb(ROOK, Them));

					// All pieces without the snipers and our queen
					BITBOARD rest = pos.get_all_pieces_bb() ^ (snipers | s);

					while (snipers)
					{
						Square sniper_sq = pop_ls1b(snipers);
						BITBOARD btw = in_between_bb(s, sniper_sq) & rest;

						// if there is space between the queen square and the slider
						// and is still left space after removing a bit
						// then there is greater than or equal to one blocker between them
						if (btw && !has_bit_after_pop(btw))
						{
							score += PIECE_SCORES.vulnerable_queen;
							break;
						}
					}
				}
			}

			return score;
		}

		template <ScoreComponent Component, Color Us>
		Score score_king(const Position &pos, Score &score)
		{
			constexpr Color Them = ~Us;
			const Square ksq = pos.square<KING>(Us);
			const CastlingRights kingside = Us & KINGSIDE;
			const CastlingRights queenside = Us & QUEENSIDE;

			auto compare_scores = [](Score a, Score b) {return a.mg < b.mg; };
			
			return score;
		}
	}

	/*
	inline int score_move(int move)
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

	int sort_move(moves* move_list)
	{
		// move scores
		std::unique_ptr<int[]> move_scores = std::make_unique<int[]>(move_list->count);

		std::cout << "\n\n";
		for (int c = 0; c < move_list->count; c++)
			move_scores[c] = score_move(move_list->moves[c]);

		for (int curr_mv = 0; curr_mv < move_list->count; curr_mv++)
			for (int next_mv = curr_mv + 1; next_mv < move_list->count; next_mv++)
			{
				// cmp current and next move scores
				if (move_scores[curr_mv] < move_scores[next_mv])
				{
					// swap the scores
					std::swap(move_scores[curr_mv], move_scores[next_mv]);
					// swap the moves
					std::swap(move_list->moves[curr_mv], move_list->moves[next_mv]);
				}
			}

		return 0;
	}

	void print_move_scores(moves* move_list)
	{
		printf("     Move scores:\n\n");

		// loop over moves within a move list
		for (int count = 0; count < move_list->count; count++)
		{
			printf("     move: ");
			print_move(move_list->moves[count]);
			printf(" score: %d\n", score_move(move_list->moves[count]));
		}
	}
	*/
}