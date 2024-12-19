#include<iostream>

#include "movegen.h"
#include "consts.h"
#include "position.h"
#include "move.h"
#include "bitboard.h"

namespace ChessEngine
{
	namespace
	{
		void add_move(Moves* move_list, Move move)
		{
			move_list->moves[move_list->count] = move;

			++move_list->count;
		}

		template<Direction D>
		void make_promotions(Moves* move_list, Square target)
		{
			add_move(move_list, Move{ target - D, target, MT_PROMOTION, PROMOTE_QUEEN });

			add_move(move_list, Move{ target - D, target, MT_PROMOTION, PROMOTE_KNIGHT });
			add_move(move_list, Move{ target - D, target, MT_PROMOTION, PROMOTE_BISHOP });
			add_move(move_list, Move{ target - D, target, MT_PROMOTION, PROMOTE_ROOK });
		}

		template<Color Us>
		void generate_pawn_moves(const Position& pos, Moves* move_list)
		{
			constexpr Color them = ~Us;
			constexpr Direction up = pawn_push_direction(Us);
			constexpr Direction up_right = (Us == WHITE ? UP_RIGHT : DOWN_RIGHT);
			constexpr Direction up_left = (Us == WHITE ? UP_LEFT : DOWN_LEFT);
			constexpr BITBOARD promotion_rank = (Us == WHITE ? Rank7_Bits : Rank2_Bits);
			constexpr BITBOARD third_rank = (Us == WHITE ? Rank3_Bits : Rank6_Bits);

			BITBOARD pawns = pos.get_pieces_bb(PAWN, Us);
			BITBOARD empty = pos.get_all_empty_squares_bb();
			BITBOARD enemies = pos.get_opponent_pieces_bb();

			BITBOARD pushed_pawns = move_to<up>(pawns & ~promotion_rank) & empty;
			BITBOARD d_pushed_pawns = move_to<up>(pushed_pawns & third_rank) & empty;
			BITBOARD promote = move_to<up>(pawns & promotion_rank) & empty;

			while (pushed_pawns)
			{
				Square target = pop_ls1b(pushed_pawns);
				add_move(move_list, Move{ target - up, target });
			}

			while (d_pushed_pawns)
			{
				Square target = pop_ls1b(d_pushed_pawns);
				add_move(move_list, Move{ target - up - up, target });
			}

			while (promote)
				make_promotions<up>(move_list, pop_ls1b(promote));

			BITBOARD left_attacks = move_to<up_left>(pawns) & enemies;
			BITBOARD right_attacks = move_to<up_right>(pawns) & enemies;

			while (left_attacks)
			{
				Square target = pop_ls1b(left_attacks);
				Square source = target - up_left;

				if (promotion_rank & source) // in case of promotions
					make_promotions<up_left>(move_list, target);
				else
					add_move(move_list, Move{ source, target });
			}

			while (right_attacks)
			{
				Square target = pop_ls1b(right_attacks);
				Square source = target - up_right;

				if (promotion_rank & source) // in case of promotions
					make_promotions<up_right>(move_list, target);
				else
					add_move(move_list, Move{ source, target });
			}

			Square ep_square = pos.ep_square();
			if (ep_square != NONE)
			{
				// if pawn is blocking a check we cannot capture it with en passant
				if (in_between_bb(pos.square<KING>(Us), pos.get_checkers()) & (ep_square + up))
					return;

				// Every pawn that is not promoting and can be captured via ep
				BITBOARD ep_pawns =
					(pawns & ~promotion_rank) & pawn_attacks_bb(them, ep_square);

				assert(ep_pawns);

				while (ep_pawns)
					add_move(move_list, Move{ pop_ls1b(ep_pawns), ep_square, MT_EN_PASSANT });
			}
		}

		template<Color Us, PieceType Pt>
		void generate_piece_moves(const Position& pos, Moves* move_list)
		{
			static_assert(Pt != PAWN, "Unsupported piece type in generate_moves()");

			BITBOARD bb = pos.get_pieces_bb(Pt, Us);
			BITBOARD all = pos.get_all_pieces_bb();
			BITBOARD not_our_sq = ~pos.get_our_pieces_bb();

			while (bb)
			{
				Square source = pop_ls1b(bb);
				BITBOARD attacks = attacks_bb_by<Pt>(source, all) & not_our_sq;

				while (attacks)
					add_move(move_list, Move{ source, pop_ls1b(attacks) }); // Quiets and captures;
			}
		}

		template<Color Us>
		void generate_castling_moves(const Position& pos, Moves* move_list)
		{
			Square ksq = pos.square<KING>(Us);
			BITBOARD all = pos.get_all_pieces_bb();

			// Gives WK and/or WQ if white
			// and BK and/or BQ if black
			if (pos.can_castle(Us & ANY) && !pos.get_attackers_to(ksq, all))
			{
				bool is_safe = true;

				for (const auto& cr : { Us & KINGSIDE, Us & QUEENSIDE })
				{
					if (cr == WK || cr == BK)
					{
						Square f1 = sq_relative_to_side(F1, Us);
						Square g1 = sq_relative_to_side(G1, Us);

						is_safe = (pos.get_attackers_to(f1, all) | pos.get_attackers_to(g1, all));
					}
					else if (cr == WQ || cr == BQ)
					{
						Square d1 = sq_relative_to_side(D1, Us);
						Square c1 = sq_relative_to_side(C1, Us);

						is_safe = (pos.get_attackers_to(d1, all) | pos.get_attackers_to(c1, all));
					}

					if (is_safe && !pos.is_castling_interrupted(cr) && pos.can_castle(cr))
						add_move(move_list, Move{ ksq, pos.castling_rook_square(cr), MT_CASTLING });
				}
			}
		}

		template<Color Us>
		void generate_all(const Position& pos, Moves* move_list)
		{
			move_list->count = 0;

			generate_pawn_moves<Us>(pos, move_list);

			generate_castling_moves<Us>(pos, move_list);

			generate_piece_moves<Us, KNIGHT>(pos, move_list);
			generate_piece_moves<Us, BISHOP>(pos, move_list);
			generate_piece_moves<Us, ROOK>(pos, move_list);
			generate_piece_moves<Us, QUEEN>(pos, move_list);
			generate_piece_moves<Us, KING>(pos, move_list);
		}

		template<Color Us>
		Moves* filtered_moves(const Position& pos, Moves* move_list)
		{
			BITBOARD pinned = pos.get_king_blockers(Us) & pos.get_our_pieces_bb();
			Square ksq = pos.square<KING>(Us);
			Moves curr[1];

			curr->count = 0;
			
			for (size_t move_i = 0; move_i < move_list->count; move_i++)
			{
				// this is for saving computational time

				// since is_legal test for en_passant illegals
				// king stepping into checks
				// or moving our piece that discoveres a check to our king
				bool is_moving_pp = pinned & move_list->moves[move_i].source_square();
				bool is_king_moving = move_list->moves[move_i].source_square() == ksq;
				bool is_en_passant = move_list->moves[move_i].move_type() == MT_EN_PASSANT;

				if ((is_moving_pp || is_king_moving || is_en_passant) && 
					!pos.is_legal(move_list->moves[move_i])) // if not legal
					continue;
				else // if legal
					add_move(curr, move_list->moves[move_i]);
			}

			return curr;
		}

	} // namespace

	Moves* generate_moves(const Position& pos, Moves* move_list)
	{
		Color us = pos.side_to_move();

		return us == WHITE
			? filtered_moves<WHITE>(pos, move_list)
			: filtered_moves<BLACK>(pos, move_list);
	}
}