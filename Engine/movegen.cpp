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

		template<Color Us, GenerationTypes Type>
		ScoredMove* generate_pawn_moves(const Position& pos, ScoredMove* move_list, BITBOARD occ)
		{
			constexpr Color them = ~Us;
			constexpr Direction up = pawn_push_direction(Us);
			constexpr Direction up_right = (Us == WHITE ? UP_RIGHT : DOWN_RIGHT);
			constexpr Direction up_left = (Us == WHITE ? UP_LEFT : DOWN_LEFT);
			constexpr BITBOARD promotion_rank = (Us == WHITE ? Rank7_Bits : Rank2_Bits);
			constexpr BITBOARD third_rank = (Us == WHITE ? Rank3_Bits : Rank6_Bits);

			BITBOARD pawns = pos.get_pieces_bb(PAWN, Us);

			BITBOARD empty = pos.get_all_empty_squares_bb();
			BITBOARD enemies = Type == GT_EVASION 
				? pos.get_checkers() : pos.get_opponent_pieces_bb();
			
			BITBOARD on7th = pawns & promotion_rank;

			if (Type != GT_CAPTURE) // Quietm Evasion or Noisy
			{
				BITBOARD pushed_pawns = move_to<up>(pawns & ~promotion_rank) & empty;
				BITBOARD d_pushed_pawns = move_to<up>(pushed_pawns & third_rank) & empty;
				BITBOARD promote = move_to<up>(on7th) & empty;

				Square target;

				if (Type == GT_EVASION) // only consider blocking squares
				{
					pushed_pawns &= occ;
					d_pushed_pawns &= occ;
					promote &= occ;
				}

				while (pushed_pawns)
				{
					target = pop_ls1b(pushed_pawns);
					*move_list++ = Move{ target - up, target };
				}

				while (d_pushed_pawns)
				{
					target = pop_ls1b(d_pushed_pawns);
					*move_list++ = Move{ target - up - up, target };
				}

				while (promote)
					move_list = make_promotions<up, false>(move_list, pop_ls1b(promote));
			}

			// Capturing and evading by capturing the attacker
			if (Type == GT_CAPTURE || Type == GT_EVASION || Type == GT_NOISY)
			{
				BITBOARD left_attacks = move_to<up_left>(pawns) & enemies;
				BITBOARD right_attacks = move_to<up_right>(pawns) & enemies;

				while (left_attacks)
				{
					Square target = pop_ls1b(left_attacks);
					Square source = target - up_left;

					if (promotion_rank & source) // in case of promotions
						move_list = make_promotions<up_left, true>(move_list, target);
					else
						*move_list++ = Move{ source, target };
				}

				while (right_attacks)
				{
					Square target = pop_ls1b(right_attacks);
					Square source = target - up_right;

					if (promotion_rank & source) // in case of promotions
						move_list = make_promotions<up_right, true>(move_list, target);
					else
						*move_list++ = Move{ source, target };
				}

				Square ep_square = pos.ep_square();
				if (ep_square != NONE)
				{
					// if pawn is blocking a check we cannot capture it with en passant
					if (Type == GT_EVASION && (occ & (ep_square + up)))
						return move_list;

					// Every pawn that is not promoting and can be captured via ep
					BITBOARD ep_pawns = 
						(pawns & ~promotion_rank) & pawn_attacks_bb(them, ep_square);

					assert(ep_pawns);

					while (ep_pawns)
						*move_list++ = Move{ pop_ls1b(ep_pawns), ep_square, MT_EN_PASSANT };
				}
			}

			return move_list;
		}

		template<Color Us, PieceType Pt>
		ScoredMove* generate_piece_moves(const Position& pos, ScoredMove* move_list, BITBOARD occ)
		{
			static_assert(Pt != PAWN && Pt != KING, "Unsupported piece type in generate_moves()");

			BITBOARD bb = pos.get_pieces_bb(Pt, Us);
			BITBOARD all = pos.get_all_pieces_bb();

			while (bb)
			{
				Square source = pop_ls1b(bb);
				BITBOARD attacks = attacks_bb_by<Pt>(source, all) & occ;

				while (attacks)
					*move_list++ = Move{ source, pop_ls1b(attacks) }; // Quiets and captures;
			}

			return move_list;
		}

		template<Color Us, GenerationTypes Type>
		ScoredMove* generate_all(const Position& pos, ScoredMove* move_list)
		{
			Square ksq = pos.square<KING>(Us);
			BITBOARD occ = 0; // Occupancies differ depending on the move type
			BITBOARD checkers_to_our_king = pos.get_checkers();

			// if double check happens then only legal option will be to make a king move
			// so skip generating other non-king moves
			if (Type != GT_EVASION || !has_bit_after_pop(checkers_to_our_king))
			{
				switch (Type)
				{
				case GT_EVASION: // Bits that are blocking the checks
					occ = in_between_bb(ksq, getLS1B(checkers_to_our_king));
					break;
				case GT_QUIET: // All the empty squares, since quiets are pawn moves
					occ = pos.get_all_empty_squares_bb();
					break;
				case GT_CAPTURE: // If move is capture then we are interested in opp's pieces
					occ = pos.get_opponent_pieces_bb();
					break;
				case GT_NOISY: // If move is noisy get every square that is not occupied by our pieces (including the ones that are occupied with opponents pieces)
					occ = ~pos.get_our_pieces_bb();
					break;
				}

				move_list = generate_pawn_moves<Us, Type>(pos, move_list, occ);

				move_list = generate_piece_moves<Us, KNIGHT>(pos, move_list, occ);
				move_list = generate_piece_moves<Us, BISHOP>(pos, move_list, occ);
				move_list = generate_piece_moves<Us, ROOK>(pos, move_list, occ);
				move_list = generate_piece_moves<Us, QUEEN>(pos, move_list, occ);
			}

			// Only difference is that if we try to evade with the king using the 
			// previously generated occupanicy we'll be
			// essentialy blocking a check to our king with our king
			BITBOARD king = attacks_bb_by<KING>(ksq) & (Type == GT_EVASION ? ~pos.get_our_pieces_bb() : occ);

			while (king)
				*move_list++ = Move{ ksq, pop_ls1b(king) };

			// Generate castle
			if ((Type == GT_QUIET || Type == GT_NOISY) && pos.can_castle(Us & ANY))
				for (CastlingRights cr : {Us& KINGSIDE, Us& QUEENSIDE})
					if (!pos.is_castling_interrupted(cr) && pos.can_castle(cr))
						*move_list++ = Move{ ksq, pos.castling_rook_square(cr), MT_CASTLING };

			return move_list;
		}

	} // namespace

	template<GenerationTypes Type>
	ScoredMove* generate_moves(const Position& pos, ScoredMove* move_list)
	{
		Color us = pos.side_to_move();

		return us == WHITE
			? generate_all<WHITE, Type>(pos, move_list)
			: generate_all<BLACK, Type>(pos, move_list);
	}

	template ScoredMove* generate_moves<GT_CAPTURE>(const Position&, ScoredMove*);
	template ScoredMove* generate_moves<GT_QUIET>(const Position&, ScoredMove*);
	template ScoredMove* generate_moves<GT_EVASION>(const Position&, ScoredMove*);
	template ScoredMove* generate_moves<GT_NOISY>(const Position&, ScoredMove*);

	// Everything else generates only pseudo-legal moves
	// GT_LEGAL generates all the legal moves in the position
	template<>
	ScoredMove* generate_moves<GT_LEGAL>(const Position& pos, ScoredMove* move_list)
	{
		Color us = pos.side_to_move();
		Square ksq = pos.square<KING>(us);
		
		ScoredMove* cur = move_list;
		
		BITBOARD pinned = pos.get_king_blockers(us) & pos.get_our_pieces_bb();
		BITBOARD checkers_to_our_king = pos.get_checkers();

		move_list = checkers_to_our_king
			? generate_moves<GT_EVASION>(pos, move_list) // either block or move out of the check
			: generate_moves<GT_NOISY>(pos, move_list); // or capture the checking piece

		while (cur != move_list)
		{
			if (cur->move_value() == 3112)
				std::cout << "\nA2A3\n";

			// if there exists a source square to a move it means that we are moving a 
			// our king blocker
			bool is_pinned_piece_moved = pinned & cur->source_square();
			bool is_king_moving = cur->source_square() == ksq;
			bool is_en_passant = cur->move_type() == MT_EN_PASSANT;
			
			if ((is_pinned_piece_moved || is_king_moving || is_en_passant) && !pos.is_legal(*cur))
				*cur = *(--move_list); // assign previous move
			else
				++cur; // increment move
		}

		return move_list;
	}
}