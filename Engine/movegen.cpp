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
		template<Direction D>
		ScoredMoves* make_promotions(ScoredMoves* move_list, Square target)
		{
			*move_list++ = Move{ target - D, target, MT_PROMOTION, PROMOTE_QUEEN };

			*move_list++ = Move{ target - D, target, MT_PROMOTION, PROMOTE_KNIGHT };
			*move_list++ = Move{ target - D, target, MT_PROMOTION, PROMOTE_BISHOP };
			*move_list++ = Move{ target - D, target, MT_PROMOTION, PROMOTE_ROOK };

			return move_list;
		}

		template<Color Us, GenerationTypes Type>
		ScoredMoves* generate_pawn_moves(const Position& pos, ScoredMoves* move_list, BITBOARD& block)
		{
			constexpr Color them = ~Us;
			constexpr Direction up = pawn_push_direction(Us);
			constexpr Direction up_right = (Us == WHITE ? UP_RIGHT : DOWN_RIGHT);
			constexpr Direction up_left = (Us == WHITE ? UP_LEFT : DOWN_LEFT);
			constexpr BITBOARD promotion_rank = (Us == WHITE ? Rank7_Bits : Rank2_Bits);
			constexpr BITBOARD third_rank = (Us == WHITE ? Rank3_Bits : Rank6_Bits);

			BITBOARD pawns = pos.get_pieces_bb(PAWN, Us);
			BITBOARD empty = pos.get_all_empty_squares_bb();

			if (Type != GT_CAPTURE) // Including QUIETS, EVASION QUIETS and ALL
			{
				BITBOARD pushed_pawns = move_to<up>(pawns & ~promotion_rank) & empty;
				BITBOARD d_pushed_pawns = move_to<up>(pushed_pawns & third_rank) & empty;

				if (Type == GT_EVADE)
				{
					pushed_pawns &= block;
					d_pushed_pawns &= block;
				}

				while (pushed_pawns)
				{
					Square target = pop_ls1b(pushed_pawns);
					*move_list++ = Move{ target - up, target };
				}

				while (d_pushed_pawns)
				{
					Square target = pop_ls1b(d_pushed_pawns);
					*move_list++ = Move{ target - up - up, target };
				}
			}

			if (Type != GT_QUIET) // Including CAPTURES, EVASIONS CAPTURES and ALL
			{
				// Promotions are under the subset of EVASION CAPTURES (Ec)
				// And can happen only when the move is not QUIET, which means that in any other case
				// such as CAPTURE, EVADE check and/or both at the same time (ALL) are the only cases where promotion can occur
				BITBOARD enemies = pos.get_opponent_pieces_bb();
				BITBOARD promote = move_to<up>(pawns & promotion_rank) & empty;

				if (Type == GT_EVADE)
				{
					// If we are evading a check, consider the only promotions that block or capture that checking piece
					promote &= block;
					enemies = block;
				}

				BITBOARD left_attacks = move_to<up_left>(pawns) & enemies;
				BITBOARD right_attacks = move_to<up_right>(pawns) & enemies;

				while (left_attacks)
				{
					Square target = pop_ls1b(left_attacks);
					Square source = target - up_left;

					if (promotion_rank & source) // in case of promotions
						move_list = make_promotions<up_left>(move_list, target);
					else
						*move_list++ = Move{ source, target };
				}

				while (right_attacks)
				{
					Square target = pop_ls1b(right_attacks);
					Square source = target - up_right;

					if (promotion_rank & source) // in case of promotions
						move_list = make_promotions<up_right>(move_list, target);
					else
						*move_list++ = Move{ source, target };
				}

				while (promote)
					move_list = make_promotions<up>(move_list, pop_ls1b(promote));

				Square ep_square = pos.ep_square();

				if (ep_square != NONE)
				{
					assert(rank_of(ep_square) == rank_relative_to_side(Us, RANK_6));

					// if pawn is blocking a check when we are supposed to be evading the check
					// we cannot capture with en passant
					if ((Type == GT_EVADE) && (block & (ep_square + up)))
						return move_list;

					// Every pawn that is not promoting and can be captured via ep
					BITBOARD ep_pawns = (pawns & ~promotion_rank) & pawn_attacks_bb(them, ep_square);

					assert(ep_pawns); // ensure that there exist ep attack 

					while (ep_pawns)
						*move_list++ = Move{ pop_ls1b(ep_pawns), ep_square, MT_EN_PASSANT };
				}
			}

			return move_list;
		}

		template<Color Us, PieceType Pt>
		ScoredMoves* generate_piece_moves(const Position& pos, ScoredMoves* move_list, BITBOARD& block_or_cap)
		{
			static_assert(Pt != PAWN && Pt != KING, "Unsupported piece type in generate_piece_moves()");

			BITBOARD bb = pos.get_pieces_bb(Pt, Us);
			BITBOARD all = pos.get_all_pieces_bb();

			while (bb)
			{
				Square source = pop_ls1b(bb);
				BITBOARD attacks = attacks_bb_by<Pt>(source, all) & block_or_cap;

				while (attacks) *move_list++ = Move{ source, pop_ls1b(attacks) };
			}

			return move_list;
		}

		template<Color Us, GenerationTypes Type>
		ScoredMoves* generate_all_pseudo_legal(const Position& pos, ScoredMoves* move_list)
		{
			const Square ksq = pos.square<KING>(Us);

			BITBOARD checkers =
				pos.get_attackers_to(ksq, pos.get_all_pieces_bb()) & pos.get_opponent_pieces_bb();

			BITBOARD block = 0ULL;

			if (Type == GT_EVADE) block =
				has_bit_after_pop(checkers) ? 0ULL : in_between_bb(ksq, get_ls1b(checkers));
			else
			{
				switch (Type)
				{
				case GT_CAPTURE:	block = pos.get_opponent_pieces_bb(); break;
				case GT_QUIET:		block = pos.get_all_empty_squares_bb(); break;
				case GT_ALL:		block = ~pos.get_our_pieces_bb(); break;
				}
			}

			move_list = generate_pawn_moves<Us, Type>(pos, move_list, block);

			move_list = generate_piece_moves<Us, KNIGHT>(pos, move_list, block);
			move_list = generate_piece_moves<Us, BISHOP>(pos, move_list, block);
			move_list = generate_piece_moves<Us, ROOK>(pos, move_list, block);
			move_list = generate_piece_moves<Us, QUEEN>(pos, move_list, block);

			// capture enemy or move to other square that is not occupied by us
			BITBOARD k_bb = attacks_bb_by<KING>(ksq) & 
				(Type == GT_EVADE ? ~pos.get_our_pieces_bb() : block);

			while (k_bb) *move_list++ = Move{ ksq, pop_ls1b(k_bb) };

			// Gives WK and/or WQ if white
			// and BK and/or BQ if black
			if ((Type == GT_QUIET || Type == GT_ALL) && !pos.is_square_attacked(ksq, ~Us) && pos.can_castle(Us & ANY))
				for (CastlingRights cr : { Us& KINGSIDE, Us& QUEENSIDE })
					if (pos.can_castle(cr) && !pos.is_castling_interrupted(cr))
						*move_list++ = Move{ ksq, pos.castling_rook_square(cr), MT_CASTLING };

			return move_list;
		}

	} // namespace

	// returns a pointer at the end of the list
	template<GenerationTypes Type>
	ScoredMoves* generate_moves(const Position& pos, ScoredMoves* move_list)
	{
		Color us = pos.side_to_move();

		return us == WHITE
			? generate_all_pseudo_legal<WHITE, Type>(pos, move_list)
			: generate_all_pseudo_legal<BLACK, Type>(pos, move_list);
	}

	// <GT_ALL>			generates all pseudo-legal captures, quiets and evasions
	// <GT_CAPTURE>		generates all pseudo-legal captures
	// <GT_QUIET>		generates all pseudo-legal quiet moves
	// <GT_EVADE>		generates all pseudo-legal check blocks or checkers captures

	template ScoredMoves* generate_moves<GT_ALL>(const Position&, ScoredMoves*);
	template ScoredMoves* generate_moves<GT_CAPTURE>(const Position&, ScoredMoves*);
	template ScoredMoves* generate_moves<GT_QUIET>(const Position&, ScoredMoves*);
	template ScoredMoves* generate_moves<GT_EVADE>(const Position&, ScoredMoves*);

	template<>
	ScoredMoves* generate_moves<GT_LEGAL>(const Position& pos, ScoredMoves* move_list)
	{
		Color us = pos.side_to_move();
		Square ksq = pos.square<KING>(us);

		ScoredMoves* legal_move_list = move_list;
		ScoredMoves pseudo_legal_moves[MAX_MOVES];
				
		ScoredMoves* end_moves = pos.get_attackers_to(ksq) & pos.get_opponent_pieces_bb()
			? generate_moves<GT_EVADE>(pos, pseudo_legal_moves)
			: generate_moves<GT_ALL>(pos, pseudo_legal_moves);

		for (ScoredMoves* move = pseudo_legal_moves; move != end_moves; ++move)
			if (pos.is_legal(*move)) *legal_move_list++ = *move;

		return legal_move_list;
	}
}