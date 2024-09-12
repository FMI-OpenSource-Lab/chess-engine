#include "movegen.h"
#include "consts.h"
#include "position.h"
#include "attacks.h"
#include "move.h"
#include "perft.h"
#include "bitboard.h"

namespace ChessEngine
{
	void generate_moves(moves* move_list)
	{
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
		Piece pawn_type = !side ? WHITE_PAWN : BLACK_PAWN;

		// side playing, for easier readability
		bool is_white = !side;

		// bitboard for white and black pawn attacks and for empty squares
		U64 empty_squares = ~occupancies[BOTH];
		U64 pawns = bitboards[pawn_type];

		// Attacks bitboard, it serves the purpose of getting the white or black attacks
		// Depends on the side playing
		U64 attacks = 0ULL;

		U64 pushed_pawnes = is_white
			? white_single_push_target(pawns, empty_squares)
			: black_single_push_target(pawns, empty_squares);

		U64 double_pushed_pawnes = is_white
			? white_double_push_target(pawns, empty_squares)
			: black_double_push_target(pawns, empty_squares);

		// promotion rank depending on the side playing
		U64 promotion_rank = is_white ? Rank8_Bits : Rank1_Bits;

		// all pushed pawns bb
		while (pushed_pawnes)
		{
			// target square that the pawn will land
			target = convert_to_square(getLS1B(pushed_pawnes));
			// shift up or down the source square depending on the side playing
			source = is_white
				? target + 8
				: target - 8;

			// promotion
			if (get_bit(promotion_rank, target))
			{
				for (Piece p = WHITE_QUEEN; p >= WHITE_KNIGHT; --p)
				{
					is_white
						? add_move(move_list,
							encode_move(source, target, pawn_type, p, 0, 0, 0, 0))
						: add_move(move_list,
							encode_move(source, target, pawn_type, (p + 6), 0, 0, 0, 0));
				}
			}
			else
			{
				// for single push
				add_move(move_list,
					encode_move(source, target, pawn_type, 0, 0, 0, 0, 0));

				// for double push
				if (double_pushed_pawnes)
				{
					target = getLS1B_square(double_pushed_pawnes);
					source = is_white ? target + 16 : target - 16;

					add_move(move_list,
						encode_move(source, target, pawn_type, 0, 0, 1, 0, 0));

					resetLSB(double_pushed_pawnes);
				}
			}

			resetLSB(pushed_pawnes);
		}

		attacks =
			all_board_pawn_attacks(pawnAttacks[side], pawns)
			& occupancies[~side];

		while (attacks)
		{
			// target square that the pawn will land
			target = getLS1B_square(attacks);

			// By appling bitwise AND to the attack table and the pawn placement bitboard
			// gets the source square index
			U64 source_pawn_attacks = pawnAttacks[~side][target] & pawns;

			while (source_pawn_attacks)
			{
				source = getLS1B_square(source_pawn_attacks);

				resetLSB(source_pawn_attacks);

				// promotion
				if (get_bit(promotion_rank, target))
				{
					for (Piece p = WHITE_QUEEN; p >= WHITE_KNIGHT; --p)
					{
						is_white
							? add_move(move_list,
								encode_move(source, target, pawn_type, p, 1, 0, 0, 0))
							: add_move(move_list,
								encode_move(source, target, pawn_type, (p + 6), 1, 0, 0, 0));
					}
				}
				else
				{
					add_move(move_list,
						encode_move(source, target, pawn_type, 0, 1, 0, 0, 0));
				}
			}

			resetLSB(attacks);
		}

		// generate enpassant moves
		if (enpassant != NONE)
		{
			// source square/s bitboard
			U64 source_bb = pawnAttacks[~side][enpassant] & pawns;

			while (source_bb)
			{
				// get the source square bit 
				source = getLS1B_square(source_bb);

				// single out the attack bits that correlate with en passant square
				U64 enpassant_attacks = pawnAttacks[side][source] & square_to_BB(enpassant);

				if (enpassant_attacks)
				{
					// get the targeted enpassant squares
					target = getLS1B_square(enpassant_attacks);

					add_move(move_list,
						encode_move(source, target, pawn_type, 0, 1, 0, 1, 0));
				}

				resetLSB(source_bb);
			}
		}
	}

	inline void castle_moves(moves* move_list)
	{
		if (!side)
		{
			// kingside castle is available
			if (castle & WK)
			{
				int free_space =
					count_bits_hamming_weight(~occupancies[BOTH] & (Rank1_Bits << 5));
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
			if (castle & WQ)
			{
				int free_space =
					count_bits_hamming_weight(~occupancies[BOTH] & (Rank1_Bits >> 4) & Rank1_Bits);

				// check that between rook and king squares are empty
				if (free_space == 3 &&
					!is_square_attacked(E1, BLACK) &&
					!is_square_attacked(D1, BLACK))
				{
					add_move(move_list,
						encode_move(E1, C1, WHITE_KING, 0, 0, 0, 0, 1));
				}
			}

		}
		else
		{
			// kingside castle is available
			if (castle & BK)
			{
				int free_space =
					count_bits_hamming_weight(~occupancies[BOTH] & (Rank8_Bits & 96));
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
			if (castle & BQ)
			{
				int free_space =
					count_bits_hamming_weight(~occupancies[BOTH] & (Rank8_Bits >> 4));

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
	}

	// King, Knight, Rook, Bishop and Queen moves generator
	inline void piece_moves(PieceType pt, moves* move_list)
	{
		Piece p = make_piece(side, pt);

		// bitboard of the current pieces of same type
		U64 bitboard = bitboards[p];
		// target squares
		U64 empty = ~occupancies[side];
		// occupanices for the opposide side
		U64 occ = occupancies[~side];
		U64 attacks;

		while (bitboard)
		{
			Square source = getLS1B_square(bitboard);

			attacks = (
				pt == KNIGHT ? knightAttacks[source]
				: pt == BISHOP ? bishopAttacks(occupancies[BOTH], source)
				: pt == ROOK ? rookAttacks(occupancies[BOTH], source)
				: pt == QUEEN ? queenAttacks(occupancies[BOTH], source)
				: kingAttacks[source]
				) & empty;

			while (attacks)
			{
				Square target = getLS1B_square(attacks);

				!get_bit(occ, target) // quiet moves
					? add_move(move_list,
						encode_move(source, target, p, 0, 0, 0, 0, 0))
					: add_move(move_list, // captures
						encode_move(source, target, p, 0, 1, 0, 0, 0));

				rm_bit(attacks, target);
			}

			rm_bit(bitboard, source);
		}
	}

	inline void add_move(moves* move_list, int move)
	{
		// store move
		move_list->moves[move_list->count] = move;

		// increment move count
		move_list->count++;
	}

	int make_move(int move, int move_flag)
	{
		// quite moves
		if (move_flag == MT_NORMAL)
		{
			// copy board state
			copy_board();

			// parse move
			int source_square = get_move_source(move);
			int target_square = get_move_target(move);

			int piece = get_move_piece(move);
			int promoted_piece = get_move_promoted(move);

			int capture_f = get_move_capture(move);
			int doublep_f = get_move_double(move);
			int enpassant_f = get_move_enpassant(move);
			int castling_f = get_move_castling(move);

			// move piece
			rm_bit(bitboards[piece], source_square);
			set_bit(bitboards[piece], target_square);

			// capture moves
			if (capture_f)
			{
				// bitboard piece index (ranges depending on side)
				Piece start_piece = !side ? BLACK_PAWN : WHITE_PAWN;
				Piece end_piece = !side ? BLACK_KING : WHITE_KING;

				// loop over bitboards opp to the current side to move
				for (Piece bb_piece = start_piece; bb_piece <= end_piece; ++bb_piece)
				{
					if (get_bit(bitboards[bb_piece], target_square))
					{
						// remove if there is a piece on the target square
						rm_bit(bitboards[bb_piece], target_square);
						break;
					}
				}
			}

			// pawn promotions
			if (promoted_piece)
			{
				// delete pawn from target
				rm_bit(bitboards[!side ? WHITE_PAWN : BLACK_PAWN], target_square);

				// set up promoted piece chessboard
				set_bit(bitboards[promoted_piece], target_square);
			}

			// enpassant
			if (enpassant_f)
			{
				// errase pawn
				(side == WHITE)
					? rm_bit(bitboards[BLACK_PAWN], (target_square + 8))
					: rm_bit(bitboards[WHITE_PAWN], (target_square - 8));
			}


			//reset enpassant
			enpassant = NONE;

			// set enpassant square if double push happened
			if (doublep_f)
				enpassant = static_cast<Square>(!side ? target_square + 8 : target_square - 8);

			if (castling_f)
			{
				switch (target_square)
				{
					// White castles kingside
				case G1:
					rm_bit(bitboards[WHITE_ROOK], H1);
					set_bit(bitboards[WHITE_ROOK], F1);
					break;
					// White castles queenside
				case C1:
					rm_bit(bitboards[WHITE_ROOK], A1);
					set_bit(bitboards[WHITE_ROOK], D1);
					break;

					// Black castles kingside
				case G8:
					rm_bit(bitboards[BLACK_ROOK], H8);
					set_bit(bitboards[BLACK_ROOK], F8);
					break;
					// Black castles queenside
				case C8:
					rm_bit(bitboards[BLACK_ROOK], A8);
					set_bit(bitboards[BLACK_ROOK], D8);
					break;
				}
			}

			// update castling rights
			castle = castle & CASTLING_RIGHTS_TABLE[source_square];
			castle = castle & CASTLING_RIGHTS_TABLE[target_square];

			// reset occupancies
			memset(occupancies, 0ULL, sizeof(occupancies));

			// update occupancies
			for (Piece p = WHITE_PAWN; p <= WHITE_KING; ++p)
			{
				occupancies[WHITE] |= bitboards[p];
				occupancies[BLACK] |= bitboards[p + 6];
			}

			occupancies[BOTH] |= occupancies[WHITE] | occupancies[BLACK];

			// change side
			side = ~side;

			// king is not exposed to check
			Square king_sq = getLS1B_square(
				(side == WHITE)
				? bitboards[BLACK_KING] :
				bitboards[WHITE_KING]);

			if (is_square_attacked(king_sq, side))
			{
				// illigal move
				restore_board();

				return 0;
			}

			// legal move
			return 1;
		}
		// capture moves
		// move is the capture
		if (get_move_capture(move))
			make_move(move, MT_NORMAL);

		// move is not a capture
		// don't make the move
		return 0;
	}

	void print_move(int move)
	{
		int promoted_piece = get_move_promoted(move);

		printf("%s%s%c",
			squareToCoordinates[get_move_source(move)],
			squareToCoordinates[get_move_target(move)],
			promoted_piece ? tolower(ascii_pieces[promoted_piece]) : '\0');
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
}