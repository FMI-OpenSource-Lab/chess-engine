#include "move.h"
#include "position.h"
#include "attacks.h"

#include <stdio.h>

namespace ChessEngine
{
	inline void generate_moves()
	{
		printf("\n");

		// source and target squares
		Square source = NONE, target = NONE, target_capture = NONE;

		// current piece's bitboard copy and attacks
		U64 current_piece_bb, attacks = 0ull, empty_squares = ~occupancies[BOTH];

		for (Piece piece = WHITE_PAWN; piece <= BLACK_KING; ++piece)
		{
			// init bb copy
			current_piece_bb = bitboards[piece];

			// generate white pawns & white king castling moves
			if (!side)
			{
				if (piece == WHITE_PAWN)
				{
					U64 pushed_pawnes = white_single_push_target(current_piece_bb, empty_squares);
					U64 double_pushed_pawnes = white_double_push_target(current_piece_bb, empty_squares);

					while (pushed_pawnes)
					{
						target = get_square(getLS1B(pushed_pawnes));
						source = target + 8;

						attacks = pawnAttacks[side][source] & occupancies[BLACK];
						target_capture = get_square(getLS1B(attacks));

						// promotion
						if (target & Rank8_Bits)
						{
							printf("promotion: %s%s qrbn\n",
								squareToCoordinates[source], squareToCoordinates[target]);

							if (attacks)
								printf("capture promotion: %s%s qrbn\n",
									squareToCoordinates[source], squareToCoordinates[target_capture]);
						}
						else
						{
							// for single push
							printf("target: %s%s\n",
								squareToCoordinates[source], squareToCoordinates[target]);

							if (attacks)
								// print messeges
								printf("target capture: %s%s\n",
									squareToCoordinates[source], squareToCoordinates[target_capture]);

							// for double push
							if (double_pushed_pawnes)
							{
								target = get_square(getLS1B(double_pushed_pawnes));
								source = target + 16;

								printf("double push: %s%s\n",
									squareToCoordinates[source], squareToCoordinates[target]);

								resetLSB(double_pushed_pawnes);
							}
						}

						resetLSB(attacks);
						resetLSB(pushed_pawnes);
					}
				}

				if (piece == WHITE_KING)
				{
					// kingside castle is available
					if (castle & WK)
					{
						int free_space = countBits(~occupancies[BOTH] & (Rank1_Bits << 5));
						// check that between rook and king squares are empty
						if (free_space == 2 &&
							!is_square_attacked(E1, BLACK) &&
							!is_square_attacked(F1, BLACK))
						{
							printf("castling move: e1g1\n");
						}
					}

					// queenside castle is available
					if (castle & WQ)
					{
						int free_space = countBits(~occupancies[BOTH] & (Rank1_Bits >> 4));

						// check that between rook and king squares are empty
						if (free_space == 3 &&
							!is_square_attacked(E1, BLACK) &&
							!is_square_attacked(D1, BLACK))
						{
							printf("castling move: e1c1\n");
						}
					}
				}
			}
			else
			{
				if (piece == BLACK_PAWN)
				{
					U64 pushed_pawnes = black_single_push_target(current_piece_bb, empty_squares);

					U64 double_pushed_pawnes = black_double_push_target(current_piece_bb, empty_squares);

					while (pushed_pawnes)
					{
						target = get_square(getLS1B(pushed_pawnes));
						source = target - 8;

						attacks = pawnAttacks[side][source] & occupancies[WHITE];
						target_capture = get_square(getLS1B(attacks));

						// promotion
						if (target & Rank1_Bits)
						{
							printf("promotion: %s%sqrbn\n",
								squareToCoordinates[source], squareToCoordinates[target]);

							if (attacks)
								printf("capture promotion: %s%s qrbn\n",
									squareToCoordinates[source], squareToCoordinates[target_capture]);
						}
						else
						{
							// print messeges
							printf("target: %s%s\n",
								squareToCoordinates[source], squareToCoordinates[target]);

							if (attacks)
								// print messeges
								printf("target capture: %s%s\n",
									squareToCoordinates[source], squareToCoordinates[target_capture]);

							if (double_pushed_pawnes)
							{
								target = get_square(getLS1B(double_pushed_pawnes));
								source = target - 16;

								printf("double push: %s%s\n",
									squareToCoordinates[source], squareToCoordinates[target]);

								resetLSB(double_pushed_pawnes);
							}
						}

						resetLSB(pushed_pawnes);
					}
				}

				if (piece == BLACK_KING)
				{
					// kingside castle is available
					if (castle & BK)
					{
						int free_space = countBits(~occupancies[BOTH] & (Rank8_Bits & 96));
						// check that between rook and king squares are empty
						if (free_space == 2 &&
							!is_square_attacked(E8, WHITE) &&
							!is_square_attacked(F8, WHITE))
						{
							printf("castling move: e8g8\n");
						}
					}

					// queenside castle is available
					if (castle & BQ)
					{
						int free_space = countBits(~occupancies[BOTH] & (Rank8_Bits >> 4));

						// check that between rook and king squares are empty
						if (free_space == 3 &&
							!is_square_attacked(E8, WHITE) &&
							!is_square_attacked(D8, WHITE))
						{
							printf("castling move: e8c8\n");
						}
					}
				}
			}
		}

		if (enpassant != NONE)
		{
			// Pick between white or black piece
			U64 piece = !side
				? bitboards[WHITE_PAWN]
				: bitboards[BLACK_PAWN];

			// en passant bitboard
			U64 ep_bb = 1ULL << enpassant;

			// source square/s bitboard
			U64 source_bb = pawnAttacks[!side][enpassant] & piece;

			// get the source square bit 
			// will get leftmost pawn if there are two pawns with en passant possibilities
			int source_square_idx = getLS1B(source_bb);
			// TODO: Deal with two en passant possibilities later

			// single out the attack bits that correlate with en passant square
			U64 enpassant_attacks = !side
				? pawnAttacks[WHITE][source_square_idx] & ep_bb
				: pawnAttacks[BLACK][source_square_idx] & ep_bb;

			if (enpassant_attacks)
			{
				// get the targeted enpassant squares
				int target_enpassant_idx = getLS1B(enpassant_attacks);

				printf("pawn enpassant capture: %s%s\n",
					squareToCoordinates[source_square_idx], squareToCoordinates[target_enpassant_idx]);
			}
		}

		// generate knight moves

		// generate bishop moves

		// generate rook moves
		// generate queen moves
		// generate king moves
	}
}