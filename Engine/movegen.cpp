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
		ScoredMove* generate_pawn_moves(const Position& pos, ScoredMove* move_list, BITBOARD target)
		{
			constexpr Color them = ~Us;
			constexpr Direction up = pawn_push_direction(Us);
			constexpr Direction up_right = (Us == WHITE ? UP_RIGHT : DOWN_RIGHT);
			constexpr Direction up_left = (Us == WHITE ? UP_LEFT : DOWN_LEFT);

			constexpr BITBOARD empty = pos.get_all_empty_squares_bb();
			constexpr BITBOARD pawns = pos.get_pieces_bb(PAWN, Us);
			constexpr BITBOARD promotion_rank = (Us == WHITE ? Rank8_Bits : Rank1_Bits);

			constexpr BITBOARD left_attacks = move_to<up_left>(pawns) & empty;
			constexpr BITBOARD right_attacks = move_to<up_right>(pawns) & empty;

			BITBOARD pushed_pawns = move_to<up>(pawns) & empty;
			
			Square source;

			while (pushed_pawns)
			{
				target = pop_ls1b(pushed_pawns);
				source = sq_relative_to_side(target - up, Us);

				if (get_bit(promotion_rank, target)) // in case of promotions
					move_list = make_promotions<up, false>(move_list, target);
				else
				{
					*move_list++ = Move{ source, target }; // single push

					if (empty & Square(target + up)) // double push
						*move_list++ = Move{ source, pushed_pawns + up };
				}
			}

			while (left_attacks)
			{
				target = pop_ls1b(left_attacks);

				if (get_bit(promotion_rank, target)) // in case of promotions
					move_list = make_promotions<up_left, true>(move_list, target);
			}

			while (right_attacks)
			{
				target = pop_ls1b(right_attacks);

				if (get_bit(promotion_rank, target)) // in case of promotions
					move_list = make_promotions<up_right, true>(move_list, target);
			}

			Square ep_square = pos.ep_square();
			if (ep_square != NONE)
			{
				BITBOARD ep_pawns = pawn_attacks_bb<~Us>(square_to_BB(ep_square));

				while (ep_pawns)
				{
					source = pop_ls1b(ep_pawns);
					*move_list++ = Move{ source, ep_square, MT_EN_PASSANT };
				}
			}

			return move_list;
		}

		template<Color Us, PieceType Pt>
		ScoredMove* generate_piece_moves(const Position& pos, ScoredMove*move_list, BITBOARD target)
		{
			static_assert(Pt != KING && Pt != PAWN, "Unsupported piece type in generate_piece_moves()");

			BITBOARD bb = pos.get_pieces_bb(Pt, Us);
			BITBOARD all = pos.get_all_pieces_bb();

			while (bb)
			{
				Square source = pop_ls1b(bb);
				BITBOARD b = attacks_bb_by<Pt>(source, all) & target;

				while (b)
					*move_list++ = Move{ source, pop_ls1b(b) };
			}

			return move_list;
		}

	} // namespace

}