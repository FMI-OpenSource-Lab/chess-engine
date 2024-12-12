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
		template<Direction D, bool Enemy>
		ScoredMove* make_promotions(ScoredMove* move_list, Square target)
		{
			*move_list++ = Move{ target - D, target, MT_PROMOTION, PROMOTE_QUEEN };

			*move_list++ = Move{ target - D, target, MT_PROMOTION, PROMOTE_KNIGHT };
			*move_list++ = Move{ target - D, target, MT_PROMOTION, PROMOTE_BISHOP };
			*move_list++ = Move{ target - D, target, MT_PROMOTION, PROMOTE_ROOK };

			return move_list;
		}

		template<Color Us>
		ScoredMove* generate_pawn_moves(const Position& pos, ScoredMove* move_list)
		{
			constexpr Color them = ~Us;
			constexpr Direction up = pawn_push_direction(Us);
			constexpr Direction up_right = (Us == WHITE ? UP_RIGHT : DOWN_RIGHT);
			constexpr Direction up_left = (Us == WHITE ? UP_LEFT : DOWN_LEFT);
			constexpr BITBOARD promotion_rank = (Us == WHITE ? Rank8_Bits : Rank1_Bits);

			BITBOARD empty = pos.get_all_empty_squares_bb();
			BITBOARD pawns = pos.get_pieces_bb(PAWN, Us);

			BITBOARD pushed_pawns = move_to<up>(pawns) & empty;

			Square source, target;

			while (pushed_pawns)
			{
				target = pop_ls1b(pushed_pawns);
				source = sq_relative_to_side(target - up, Us);

				if (promotion_rank & target) // in case of promotions
				{
					move_list = make_promotions<up, false>(move_list, target);
				}
				else
				{
					*move_list++ = Move{ source, target }; // single push

					if (empty & Square(target + up)) // double push
						*move_list++ = Move{ source, target + up };
				}
			}

			BITBOARD enemy = pos.get_opponent_pieces_bb();

			BITBOARD left_attacks = move_to<up_left>(pawns) & enemy;
			BITBOARD right_attacks = move_to<up_right>(pawns) & enemy;

			while (left_attacks)
			{
				target = pop_ls1b(left_attacks);

				if (promotion_rank & target) // in case of promotions
					move_list = make_promotions<up_left, true>(move_list, target);
				else
					*move_list++ = Move{ Square(target - up_left), target };
			}

			while (right_attacks)
			{
				target = pop_ls1b(right_attacks);

				if (promotion_rank & target) // in case of promotions
					move_list = make_promotions<up_right, true>(move_list, target);
				else
					*move_list++ = Move{ Square(target - up_right), target };
			}

			Square ep_square = pos.ep_square();
			if (ep_square != NONE)
			{
				BITBOARD ep_pawns = pawn_attacks_bb<~Us>(square_to_BB(ep_square)) & pawns;

				while (ep_pawns)
					*move_list++ = Move{ pop_ls1b(ep_pawns), ep_square, MT_EN_PASSANT };
			}

			return move_list;
		}

		template<Color Us, PieceType Pt>
		ScoredMove* generate_piece_moves(const Position& pos, ScoredMove* move_list)
		{
			static_assert(Pt != PAWN, "Unsupported piece type in generate_moves()");

			BITBOARD bb = pos.get_pieces_bb(Pt, Us);
			BITBOARD all = pos.get_all_pieces_bb();
			BITBOARD not_our = ~pos.get_pieces_bb(Us); // every square except the squares occupied by Us

			while (bb)
			{
				Square source = pop_ls1b(bb);
				BITBOARD attacks = attacks_bb_by<Pt>(source, all) & not_our;

				while (attacks)
					*move_list++ = Move{ source, pop_ls1b(attacks) }; // Quiets and captures;
			}

			return move_list;
		}

		template<Color Us>
		ScoredMove* generate_castling_moves(const Position& pos, ScoredMove* move_list)
		{
			Square ksq = pos.square<KING>(Us);

			if (pos.can_castle(Us & ANY))
				for (CastlingRights cr : {Us& KINGSIDE, Us& QUEENSIDE})
					if (!pos.is_castling_interrupted(cr) && pos.can_castle(cr))
						*move_list++ = Move{ ksq, pos.castling_rook_square(cr), MT_CASTLING };

			return move_list;
		}

		template<Color Us>
		ScoredMove* generate_all(const Position& pos, ScoredMove* move_list)
		{
			move_list = generate_pawn_moves<Us>(pos, move_list);

			move_list = generate_castling_moves<Us>(pos, move_list);

			move_list = generate_piece_moves<Us, KNIGHT>(pos, move_list);
			move_list = generate_piece_moves<Us, BISHOP>(pos, move_list);
			move_list = generate_piece_moves<Us, ROOK>(pos, move_list);
			move_list = generate_piece_moves<Us, QUEEN>(pos, move_list);
			move_list = generate_piece_moves<Us, KING>(pos, move_list);

			return move_list;
		}

	} // namespace

	ScoredMove* generate_moves(const Position& pos, ScoredMove* move_list)
	{
		Color us = pos.side_to_move();

		return us == WHITE
			? generate_all<WHITE>(pos, move_list)
			: generate_all<BLACK>(pos, move_list);
	}
}