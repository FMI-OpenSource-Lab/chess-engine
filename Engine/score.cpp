#include "score.h"

#include "defs.h"
#include "move.h"
#include "consts.h"
#include "position.h"
#include "bitboard.h"

#include <algorithm>
#include <memory>

namespace ChessEngine
{
	int evaluate()
	{
		Value score = 0;
		U64 bitboard;

		Piece piece;
		Square square;

		for (Piece bb_p = WHITE_PAWN; bb_p <= BLACK_KING; ++bb_p)
		{
			bitboard = bitboards[bb_p];

			while (bitboard)
			{
				piece = bb_p;
				square = getLS1B_square(bitboard);

				score += MATERIAL_SCORE[piece];

				switch (piece)
				{
					// eval white pieces position
				case WHITE_PAWN:	score += PAWN_SCORE[square]; break;
				case WHITE_KNIGHT:	score += KNIGHT_SCORE[square]; break;
				case WHITE_BISHOP:	score += BISHOP_SCORE[square]; break;
				case WHITE_ROOK:	score += ROOK_SCORE[square]; break;
				case WHITE_KING:	score += KING_SCORE[square]; break;

					// eval black pieces position
				case BLACK_PAWN:	score -= PAWN_SCORE[MIRROR_SCORE[square]]; break;
				case BLACK_KNIGHT:	score -= KNIGHT_SCORE[MIRROR_SCORE[square]]; break;
				case BLACK_BISHOP:	score -= BISHOP_SCORE[MIRROR_SCORE[square]]; break;
				case BLACK_ROOK:	score -= ROOK_SCORE[MIRROR_SCORE[square]]; break;
				case BLACK_KING:	score -= KING_SCORE[MIRROR_SCORE[square]]; break;
				}

				rm_bit(bitboard, square);
			}
		}

		//return final eval based on side
		return !side ? score : -score;
	}

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
}