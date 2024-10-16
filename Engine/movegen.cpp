#include "movegen.h"
#include "consts.h"
#include "position.h"
#include "move.h"
#include "bitboard.h"

namespace ChessEngine
{
	// Generate promotions
	template<GenerationTypes type, Direction D, bool Enemy>
	Move* make_promotions(Move* move_list, Square target)
	{
		bool all = type == GT_EVASION || type == GT_NON_EVATION;

		if (type == GT_CAPTURE || all)
			*move_list++ = Move::make<MT_PROMOTION>(target + D, target, QUEEN);

		if ((type == GT_CAPTURE || Enemy) || (type == GT_QUIET || !Enemy) || all)
		{
			*move_list++ = Move::make<MT_PROMOTION>(target + D, target, ROOK);
			*move_list++ = Move::make<MT_PROMOTION>(target + D, target, BISHOP);
			*move_list++ = Move::make<MT_PROMOTION>(target + D, target, KNIGHT);
		}

		return move_list;
	}

	// Generate pawn moves
	template<GenerationTypes type, Color us>
	Move* generate_pawn_moves(Move* move_list, const Position& pos, BITBOARD target)
	{
		constexpr Direction push_dir = pawn_push_direction(us);
		constexpr Direction u_r = us == WHITE ? UP_RIGHT : DOWN_RIGHT;
		constexpr Direction u_l = us == WHITE ? UP_LEFT : DOWN_LEFT;

		BITBOARD empty_squares = pos.get_all_empty_squares_bb();
		BITBOARD pawns = pos.get_pieces_bb(PAWN, us);
		BITBOARD enemies = pos.get_opponent_pieces_bb();
		// Double push rank
		BITBOARD d_rank = us == WHITE ? Rank4_Bits : Rank5_Bits;
		// Promotion rank
		BITBOARD p_rank = us == WHITE ? Rank8_Bits : Rank1_Bits;

		// single and double pawn pushes, without promotions
		if (type != GT_CAPTURE)
		{
			BITBOARD single_pushed_pawns = move_to<push_dir>(pawns) & empty_squares;
			BITBOARD double_pushed_pawns = move_to<push_dir>(single_pushed_pawns) & empty_squares & d_rank;

			// Only blocking squares
			if (type == GT_EVASION)
			{
				single_pushed_pawns &= target;
				double_pushed_pawns &= target;
			}

			while (single_pushed_pawns)
			{
				Square target = pop_ls1b(single_pushed_pawns);
				*move_list++ = Move(target + push_dir, target);
			}

			while (double_pushed_pawns)
			{
				Square target = pop_ls1b(double_pushed_pawns);
				*move_list++ = Move(target + push_dir + push_dir, target);
			}
		}

		// Promotions and underpromotions
		if (pawns & p_rank)
		{
			// capture bitboard
			BITBOARD capture_r = move_to<u_r>(pawns) & enemies;
			BITBOARD capture_l = move_to<u_l>(pawns) & enemies;
			BITBOARD push = move_to<push_dir>(pawns) & empty_squares;

			if (type == GT_EVASION) push &= target;

			while (capture_r)
				move_list = make_promotions<type, u_r, true>(move_list, pop_ls1b(capture_r));

			while (capture_l)
				move_list = make_promotions<type, u_l, true>(move_list, pop_ls1b(capture_l));

			while (push)
				move_list = make_promotions<type, push_dir, false>(move_list, pop_ls1b(push));
		}

		// Standart and en passant captures
		if (type == GT_CAPTURE || type == GT_EVASION || GT_NON_EVATION)
		{
			BITBOARD seventh_rank = us == WHITE ? Rank7_Bits : Rank3_Bits;
			BITBOARD pawns_not_on_seventh = pawns & ~seventh_rank;

			BITBOARD push_r = move_to<u_r>(pawns_not_on_seventh) & enemies;
			BITBOARD push_l = move_to<u_l>(pawns_not_on_seventh) & enemies;

			// Add moves to the list
			while (push_r)
			{
				Square target = pop_ls1b(push_r);
				*move_list++ = Move(target + u_r, target);
			}

			while (push_l)
			{
				Square target = pop_ls1b(push_l);
				*move_list++ = Move(target + u_l, target);
			}

			// Ep square exists
			if (pos.ep_square() != NONE)
			{
				Square ep_square = pos.ep_square();

				// Make sure that en passant can be done on either Rank 6 or Rank 3
				assert(rank_of(ep_square) == Rank(RANK_6 ^ (us * 7)));

				// An en passant capture cannot be discovering a check to our king
				if (type == GT_EVASION && (target & (ep_square + push_dir)))
					return move_list;

				push_r = pawns_not_on_seventh & pawn_attacks_bb(~us, ep_square);

				// check if empty
				assert(push_r);

				while (push_r)
					*move_list++ = Move::make<MT_EN_PASSANT>(pop_ls1b(push_r), ep_square);
			}
		}

		return move_list;
	}

	template<Color us, PieceType pt>
	Move* generate_moves(Move* move_list, const Position& pos, BITBOARD target)
	{
		assert(pt != KING && pt != PAWN);

		BITBOARD pieces = pos.get_pieces_bb(pt, us);

		while (pieces)
		{
			Square source = pop_ls1b(pieces);
			BITBOARD attacks = attacks_bb_by<pt>(source, pos.get_all_pieces_bb()) & target;

			while (attacks)
				*move_list++ = Move(source, pop_ls1b(attacks));
		}

		return move_list;
	}

	// Generate all moves
	template<Color us, GenerationTypes type>
	Move* generate_all(Move* move_list, const Position& pos)
	{
		// unsupported type
		assert(type != GT_LEGAL);

		Square king_square = pos.square<KING>(us);
		BITBOARD b;
		BITBOARD checking_pieces =
			pos.get_attackers_to(pos.square<KING>(us)) & pos.get_pieces_bb(~us);

		// Skip generating non-king moves when king is in double check
		if (type != GT_EVASION || !has_bit_after_pop(checking_pieces))
		{
			if (type == GT_EVASION)
				b = in_between_bb(king_square, getLS1B_square(checking_pieces));
			else if (type == GT_NON_EVATION)
				b = ~pos.get_our_pieces_bb();
			else if (type == GT_CAPTURE)
				b = pos.get_opponent_pieces_bb();
			else // Quiets
				b = ~pos.get_all_pieces_bb();

			move_list = generate_pawn_moves<type, us>(move_list, pos, b);
			move_list = generate_moves<us, KNIGHT>(move_list, pos, b);
			move_list = generate_moves<us, BISHOP>(move_list, pos, b);
			move_list = generate_moves<us, ROOK>(move_list, pos, b);
			move_list = generate_moves<us, QUEEN>(move_list, pos, b);
		}

		BITBOARD pc = type == GT_EVASION ? ~pos.get_our_pieces_bb() : b;
		BITBOARD bb = attacks_bb_by<KING>(king_square) & pc;

		// add evation moves to the list
		while (bb)
			*move_list++ = Move(king_square, pop_ls1b(bb));

		if ((type == GT_QUIET || type == GT_NON_EVATION) && pos.can_castle(us & ANY))
			for (auto cr : { us & KINGSIDE, us & QUEENSIDE })
			{
				if (!pos.is_castling_prevented(cr) && pos.can_castle(cr))
				{
					*move_list++ = Move::make<MT_CASTLING>(king_square, pos.castling_rook_square(cr));
				}
			}

		return move_list;
	}

	// Generate CAPTURE, QUIET, EVATION, NON-EVATION and LEGALs
	template<GenerationTypes type>
	Move* generate(Move* move_list, const Position& pos)
	{
		assert(type != GT_LEGAL);

		Color us = pos.side_to_move();

		return us == WHITE
			? generate_all<WHITE, type>(move_list, pos)
			: generate_all<BLACK, type>(move_list, pos);
	}

	template Move* generate<GT_CAPTURE>(Move* move_list, const Position& pos);
	template Move* generate<GT_QUIET>(Move* move_list, const Position& pos);
	template Move* generate<GT_EVASION>(Move* move_list, const Position& pos);
	template Move* generate<GT_NON_EVATION>(Move* move_list, const Position& pos);

	template<>
	Move* generate<GT_LEGAL>(Move* moveList, const Position& pos)
	{
		Color us = pos.side_to_move();
		BITBOARD pinned = pos.get_blocking_pieces(us) & pos.get_our_pieces_bb();
		Square king_square = pos.square<KING>(us);
		Move* current = moveList;

		BITBOARD checking_pieces =
			pos.get_attackers_to(pos.square<KING>(us)) & pos.get_opponent_pieces_bb();

		moveList = checking_pieces
			? generate<GT_EVASION>(moveList, pos)
			: generate<GT_NON_EVATION>(moveList, pos);

		while (current != moveList)
		{
			if (((pinned & current->source_square()) || current->source_square() == king_square || current->move_type() == MT_EN_PASSANT)
				&& !pos.is_legal(*current))
			{
				*current = *(--moveList);
			}
			else
				++current;
		}

		return moveList;
	}
}