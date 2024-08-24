#include "move.h"
#include "position.h"
#include "attacks.h"

#include <stdio.h>

namespace ChessEngine
{
	void GenerateMoves::generate()
	{
		printf("\n");

		// generate pawn moves
		pawn_moves();

		// generate king moves
		king_moves();

		// generate knight moves
		knight_moves();

		// generate bishop moves
		bishop_moves();

		// generate rook moves
		rook_moves();

		// generate queen moves
		queen_moves();
	}

	void GenerateMoves::pawn_moves()
	{
		// target and source squares
		Square target, source;

		// side playing, for easier readability
		bool is_white = !side;

		// bitboard for white and black pawn attacks and for empty squares
		U64 empty_squares = ~occupancies[BOTH];
		U64 pawn_attacks = !side ? bitboards[WHITE_PAWN] : bitboards[BLACK_PAWN];

		// Attacks bitboard, it serves the purpose of getting the white or black attacks
		// Depends on the side playing
		U64 attacks = 0ULL;

		U64 pushed_pawnes = is_white
			? white_single_push_target(pawn_attacks, empty_squares)
			: black_single_push_target(pawn_attacks, empty_squares);

		U64 double_pushed_pawnes = is_white
			? white_double_push_target(pawn_attacks, empty_squares)
			: black_double_push_target(pawn_attacks, empty_squares);

		// promotion rank depending on the side playing
		U64 promotion_rank = is_white ? Rank8_Bits : Rank1_Bits;

		while (pushed_pawnes)
		{
			// target square that the pawn will land
			target = get_square(getLS1B(pushed_pawnes));
			// shift up or down the source square depending on the side playing
			source = is_white
				? target + 8
				: target - 8;

			// promotion
			if (target & promotion_rank)
			{
				printf("promotion: %s%s qrbn\n",
					squareToCoordinates[source], squareToCoordinates[target]);
			}
			else
			{
				// for single push
				printf("target: %s%s\n",
					squareToCoordinates[source], squareToCoordinates[target]);

				// for double push
				if (double_pushed_pawnes)
				{
					target = get_square(getLS1B(double_pushed_pawnes));
					source = is_white ? target + 16 : target - 16;

					printf("double push: %s%s\n",
						squareToCoordinates[source], squareToCoordinates[target]);

					resetLSB(double_pushed_pawnes);
				}
			}

			resetLSB(pushed_pawnes);
		}

		attacks =
			all_board_pawn_attacks(pawnAttacks[side], pawn_attacks)
			& occupancies[~side];

		while (attacks)
		{
			// target square that the pawn will land
			target = get_square(getLS1B(attacks));

			// By appling bitwise AND to the attack table and the pawn placement bitboard
			// gets the source square index
			int source_square = is_white
				? getLS1B(pawnAttacks[BLACK][target] & pawn_attacks)
				: getLS1B(pawnAttacks[WHITE][target] & pawn_attacks);

			// promotion
			if (target & promotion_rank)
			{
				printf("promotion capture: %s%s qrbn\n",
					squareToCoordinates[source_square], squareToCoordinates[target]);
			}
			else
			{
				// print messeges
				printf("target capture: %s%s\n",
					squareToCoordinates[source_square], squareToCoordinates[target]);
			}

			resetLSB(attacks);
		}

		// generate enpassant moves
		if (enpassant != NONE)
		{
			// en passant bitboard
			U64 ep_bb = 1ULL << enpassant;

			// source square/s bitboard
			U64 source_bb = pawnAttacks[~side][enpassant] & pawn_attacks;

			// get the source square bit 
			// will get leftmost pawn if there are two pawns with en passant possibilities
			int source_square_idx = getLS1B(source_bb);
			// TODO: Deal with two en passant possibilities later

			// single out the attack bits that correlate with en passant square
			U64 enpassant_attacks = pawnAttacks[side][source_square_idx] & ep_bb;

			if (enpassant_attacks)
			{
				// get the targeted enpassant squares
				int target_enpassant_idx = getLS1B(enpassant_attacks);

				printf("pawn enpassant capture: %s%s\n",
					squareToCoordinates[source_square_idx],
					squareToCoordinates[target_enpassant_idx]);
			}
		}
	}

	void GenerateMoves::king_moves()
	{
		// kingside castle is available
		if ((castle & WK) && !side)
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
		if ((castle & WQ) && !side)
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

		// kingside castle is available
		if ((castle & BK) && side)
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
		if ((castle & BQ) && side)
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

	void GenerateMoves::knight_moves()
	{
		U64 knight_bb = !side ? bitboards[WHITE_KNIGHT] : bitboards[BLACK_KNIGHT];
		U64 empty = !side ? ~occupancies[WHITE] : ~occupancies[BLACK];
		U64 attacks;

		while (knight_bb)
		{
			int source_square_index = getLS1B(knight_bb);

			attacks = knightAttacks[source_square_index] & empty;

			while (attacks)
			{
				int target = getLS1B(attacks);

				// TODO: Make quiet move and capture move
				// make bitboard mask of every knight capture move
			}
		}

	}

	void GenerateMoves::bishop_moves()
	{

	}

	void GenerateMoves::rook_moves()
	{

	}

	void GenerateMoves::queen_moves()
	{

	}
}