#include "move.h"
#include "position.h"
#include "bitboard.h"

namespace ChessEngine
{
	inline void generate_moves()
	{
		// source and target squares
		Square source, target;

		// current piece's bitboard copy and attacks
		U64 bitboard, attacks;

		for (Piece piece = WHITE_PAWN; piece <= BLACK_KING; ++piece)
		{
			// init bb copy
			bitboard = bitboards[piece];

			// generate white pawns & white king castling moves

			if (!side)
			{
				if (piece == WHITE_PAWN)
				{
					while (bitboard)
					{
						source = get_square(getLS1B(bitboard));

						// printf("white pawn: %s\n", squareToCoordinates[source]);

						target = source - 8;

						if (!(target < A8) && !get_bit(occupancies[BOTH], target))
						{
							// pawn promotion
							if (source >= A7 && source <= H7)
							{
								printf("pawn promotion: %s%sq\n", squareToCoordinates[source], squareToCoordinates[target]);
								printf("pawn promotion: %s%sr\n", squareToCoordinates[source], squareToCoordinates[target]);
								printf("pawn promotion: %s%sb\n", squareToCoordinates[source], squareToCoordinates[target]);
								printf("pawn promotion: %s%sn\n", squareToCoordinates[source], squareToCoordinates[target]);

							}
							else
							{
								// one square ahead
								printf("pawn push: %s%s\n", 
									squareToCoordinates[source], squareToCoordinates[target]);

								// two squares ahead
								if ((source >= A2 && source <= H2) 
									&& !get_bit(occupancies[BOTH], target - 8))
								{
									printf("double pawn push: %s%s\n",
										squareToCoordinates[source], squareToCoordinates[target - 8]);
								}
							}
						}

						rm_bit(bitboard, source);
					}
				}
			}

			// generate black pawns & black king castling moves

			// generate knight moves

			// generate bishop moves

			// generate rook moves
			// generate queen moves
			// generate king moves
		}
	}
}