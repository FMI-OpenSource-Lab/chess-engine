#include "score.h"

#include "defs.h"
#include "consts.h"
#include "position.h"
#include "bitboard.h"

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

				resetLSB(bitboard);
			}
		}

		//return final eval based on side
		return !side ? score : -score;
	}
}