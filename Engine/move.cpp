#include "move.h"
#include "position.h"
#include "attacks.h"

#include <stdio.h>

namespace ChessEngine
{
	void generate_moves(moves* move_list)
	{
		printf("\n");

		move_list->count = 0;

		// generate pawn moves
		pawn_moves(move_list);

		// generate castle moves
		castle_moves(move_list);

		// generate knight moves
		piece_moves(KNIGHT, move_list);

		// generate bishop moves
		piece_moves(BISHOP, move_list);

		// generate rook moves
		piece_moves(ROOK, move_list);

		// generate queen moves
		piece_moves(QUEEN, move_list);

		// generate queen moves
		piece_moves(KING, move_list);
	}

	inline void pawn_moves(moves* move_list)
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

		Piece piece = !side ? WHITE_PAWN : BLACK_PAWN;
		
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
				for (Piece p = WHITE_QUEEN; p >= WHITE_KNIGHT; --p)
				{
					if (!side)
						add_move(move_list,
							encode_move(source, target, piece, p, 0, 0, 0, 0));
					else
						add_move(move_list,
							encode_move(source, target, piece, p + 6, 0, 0, 0, 0));
				}
			}
			else
			{
				// for single push
				add_move(move_list,
					encode_move(source, target, piece, 0, 0, 0, 0, 0));

				// for double push
				if (double_pushed_pawnes)
				{
					target = get_square(getLS1B(double_pushed_pawnes));
					source = is_white ? target + 16 : target - 16;

					add_move(move_list,
						encode_move(source, target, piece, 0, 0, 1, 0, 0));


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
				for (Piece p = WHITE_QUEEN; p >= WHITE_KNIGHT; --p)
				{
					if (!side)
						add_move(move_list,
							encode_move(source_square, target, piece, p, 1, 0, 0, 0));
					else
						add_move(move_list,
							encode_move(source_square, target, piece, p + 6, 1, 0, 0, 0));
				}
			}
			else
			{
				add_move(move_list,
					encode_move(source_square, target, piece, 0, 1, 0, 0, 0));
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

				add_move(move_list,
					encode_move(source_square_idx, target_enpassant_idx, piece, 0, 1, 0, 1, 0));
			}
		}
	}

	inline void castle_moves(moves* move_list)
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
				add_move(move_list,
					encode_move(E1, G1, WHITE_KING, 0, 0, 0, 0, 1));
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
				add_move(move_list,
					encode_move(E1, C1, WHITE_KING, 0, 0, 0, 0, 1));
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
				add_move(move_list,
					encode_move(E8, G8, BLACK_KING, 0, 0, 0, 0, 1));
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
				add_move(move_list,
					encode_move(E8, C8, BLACK_KING, 0, 0, 0, 0, 1));
			}
		}
	}

	// King, Knight, Rook, Bishop and Queen moves generator
	inline void piece_moves(PieceType pt, moves* move_list)
	{
		Piece p = static_cast<Piece>(!side ? pt - 1 : pt + 5);

		// bitboard containing the pieces
		U64 bitboard = bitboards[p];
		// target squares
		U64 empty = ~occupancies[side];
		// occupanices for the opposide side
		U64 occ = occupancies[~side];
		U64 attacks;

		while (bitboard)
		{
			Square source = getLS1B_square(bitboard);

			attacks =
				pt == KNIGHT ? knightAttacks[source]
				: pt == BISHOP ? bishopAttacks(occupancies[BOTH], source)
				: pt == ROOK ? rookAttacks(occupancies[BOTH], source)
				: pt == QUEEN ? queenAttacks(occupancies[BOTH], source)
				: kingAttacks[source];

			attacks &= empty;

			while (attacks)
			{
				int target = getLS1B(attacks);

				// quiet moves
				if (!get_bit(occ, target))
				{
					add_move(move_list,
						encode_move(source, target, p, 0, 0, 0, 0, 0));
				}
				else
				{
					add_move(move_list,
						encode_move(source, target, p, 0, 1, 0, 0, 0));
				}

				resetLSB(attacks);
			}

			resetLSB(bitboard);
		}
	}

	void print_move(int move)
	{
		printf("%s%s%c\n",
			squareToCoordinates[get_move_source(move)],
			squareToCoordinates[get_move_target(move)],
			ascii_pieces[get_move_promoted(move)]);
	}

	inline void print_move_list(moves* move_list)
	{
		if (!move_list->count)
		{
			printf("\n	 No moves in the move list\n");
			return;
		}

		printf("\n	 move     piece   capture   double flag   enpassant flag   castling flag\n\n");

		for (size_t move_count = 0; move_count < move_list->count; move_count++)
		{
			int move = move_list->moves[move_count];
			int promoted_index = get_move_promoted(move);

			printf("	 %s%s%c    %c       %d         %d             %d                %d\n",
				squareToCoordinates[get_move_source(move)],
				squareToCoordinates[get_move_target(move)],
				!promoted_index ? ' ' : ascii_pieces[promoted_index],
				ascii_pieces[get_move_piece(move)],
				get_move_capture(move) ? 1 : 0,
				get_move_double(move) ? 1 : 0,
				get_move_enpassant(move) ? 1 : 0,
				get_move_castling(move) ? 1 : 0
			);
		}

		std::cout << "\n	 Total number of moves: " << move_list->count << "\n\n";
	}

	inline void add_move(moves* move_list, int move)
	{
		// store move
		move_list->moves[move_list->count] = move;

		// increment move count
		move_list->count++;
	}
}