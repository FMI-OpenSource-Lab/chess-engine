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
	void Eval::init_eval_info(EvalInfo &ei, const Position &pos)
	{
		// Initialize the evaluation info structure
		ei.game_phase = 0;

		ei.total_king_ring_attacks[WHITE] = ei.total_king_ring_attacks[BLACK] = 0;

		ei.king_squares[WHITE] = pos.square<KING>(WHITE);
		ei.king_squares[BLACK] = pos.square<KING>(BLACK);

		ei.pawns[WHITE] = pos.get_pieces_bb(PAWN, WHITE);
		ei.pawns[BLACK] = pos.get_pieces_bb(PAWN, BLACK);

		ei.pieces[WHITE] = pos.get_pieces_bb(WHITE);
		ei.pieces[BLACK] = pos.get_pieces_bb(BLACK);

		ei.pawn_attacks[WHITE] = pawn_attacks_bb<WHITE>(ei.pawns[WHITE]);
		ei.pawn_attacks[BLACK] = pawn_attacks_bb<BLACK>(ei.pawns[BLACK]);

		for(PieceType pt = PAWN; pt <= QUEEN; ++pt)
		{
			ei.piece_counts[WHITE][pt] = count_bits(pos.get_pieces_bb(pt, WHITE));
			ei.piece_counts[BLACK][pt] = count_bits(pos.get_pieces_bb(pt, BLACK));

			ei.piece_relative_occupancies[WHITE][pt] = pos.get_all_pieces_bb();
			ei.piece_relative_occupancies[BLACK][pt] = pos.get_all_pieces_bb();
		}

		BITBOARD w_diagonal = pos.get_pieces_bb(BISHOP, WHITE) | pos.get_pieces_bb(QUEEN, WHITE);
		BITBOARD b_diagonal = pos.get_pieces_bb(BISHOP, BLACK) | pos.get_pieces_bb(QUEEN, BLACK);

		BITBOARD w_horizontal = pos.get_pieces_bb(ROOK, WHITE) | pos.get_pieces_bb(QUEEN, WHITE);
		BITBOARD b_horizontal = pos.get_pieces_bb(ROOK, BLACK) | pos.get_pieces_bb(QUEEN, BLACK);

		ei.piece_relative_occupancies[WHITE][BISHOP] ^= w_diagonal;
		ei.piece_relative_occupancies[BLACK][BISHOP] ^= b_diagonal;
		
		ei.piece_relative_occupancies[WHITE][ROOK] ^= w_horizontal;
		ei.piece_relative_occupancies[BLACK][ROOK] ^= b_horizontal;

		ei.piece_relative_occupancies[WHITE][QUEEN] ^= w_diagonal | w_horizontal;
		ei.piece_relative_occupancies[BLACK][QUEEN] ^= b_diagonal | b_horizontal;
	}

	// Calcuates the material score of the position depending on the side to move
	template <Color Us>
	Value Eval::simple_evaluation(const Position &pos)
	{
		// Calculates only the material balance
		return PAWN_VALUE * (pos.count<PAWN>(Us) - pos.count<PAWN>(~Us)) + (pos.material_value(Us) - pos.material_value(~Us));
	}

	// Evaluates the position using the material score and positional score
	Value Eval::evaluate(const Position &pos)
	{
		Color us = pos.side_to_move();

		return simple_evaluation<WHITE>(pos) + pos.pst_value(us) - pos.pst_value(~us);
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