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
		ScoredMove* generate_piece_moves(const Position& pos, ScoredMove* move_list)
		{
			static_assert(Pt != PAWN, "Unsupported piece type in generate_moves()");

			BITBOARD bb = pos.get_pieces_bb(Pt, Us);
			BITBOARD all = pos.get_all_pieces_bb();
			BITBOARD empty = pos.get_all_empty_squares_bb();

			while (bb)
			{
				Square source = pop_ls1b(bb);
				BITBOARD attacks = attacks_bb_by<Pt>(source, all) & empty;

				while (attacks)
					*move_list++ = Move{ source, pop_ls1b(attacks) }; // Quiets and captures
			}

			return move_list;
		}

		template<Color Us>
		ScoredMove* generate_castling_moves(const Position& pos, ScoredMove* move_list)
		{
			Square ksq = pos.square<KING>(Us);

			for (CastlingRights cr : {Us& KINGSIDE, Us& QUEENSIDE})
				if (!pos.is_castling_interrupted(cr) && pos.can_castle(cr))
					*move_list++ = Move{ ksq, pos.castling_rook_square(cr), MT_CASTLING };

			return move_list;
		}

		template<Color Us, GenerationTypes Type>
		ScoredMove* generate_all(const Position& pos, ScoredMove* move_list)
		{
			static_assert(Type != GT_LEGAL, "Unsupported type in generate_all()");

			move_list = generate_pawn_moves<Us, Type>(pos, move_list);

			move_list = generate_castling_moves<Us>(pos, move_list);

			move_list = generate_moves<Us, KNIGHT>(pos, move_list);
			move_list = generate_moves<Us, BISHOP>(pos, move_list);
			move_list = generate_moves<Us, ROOK>(pos, move_list);
			move_list = generate_moves<Us, QUEEN>(pos, move_list);
			move_list = generate_moves<Us, KING>(pos, move_list);

			return move_list;
		}

	} // namespace

	template<GenerationTypes Type>
	ScoredMove* generate_moves(const Position& pos, ScoredMove* move_list)
	{
		static_assert(Type != GT_LEGAL, "Unsupported type in generate()");

		Color us = pos.side_to_move();

		return us == WHITE
			? generate_all<WHITE, Type>(pos, move_list)
			: generate_all<BLACK, Type>(pos, move_list);
	}

	template<>
	ScoredMove* generate_moves<GT_LEGAL>(const Position& pos, ScoredMove* move_list)
	{
		Color us = pos.side_to_move();
		Square ksq = pos.square<KING>(us);
		ScoredMove* current = move_list;
		BITBOARD pinned = pos.get_king_blockers(us) & pos.get_our_pieces_bb();
		BITBOARD checkers = pos.get_attackers_to(ksq) & pos.get_opponent_pieces_bb();

		move_list = generate_moves<GT_LEGAL>(pos, move_list);

		while (current != move_list)
		{
			if (((pinned & current->source_square())
				|| current->source_square() == ksq
				|| current->move_type() == MT_EN_PASSANT)
				&& !pos.is_legal(*current)) {

				*current = *(--move_list);
			}
			else
				++current;
		}

		return move_list;
	}
}