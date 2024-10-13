#include "movegen.h"
#include "consts.h"
#include "position.h"
#include "move.h"
#include "perft.h"
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
	Move* generate_pawn_moves(Move* move_list, Position& pos, BITBOARD target)
	{
		Color them = ~us;

		BITBOARD T7th_bb = (us == WHITE ? Rank7_Bits : Rank2_Bits);
		BITBOARD T3rd_bb = (us == WHITE ? Rank3_Bits : Rank6_Bits);

		Direction up = pawn_push_direction(us);
		Direction up_left = (us == WHITE ? UP_LEFT : DOWN_LEFT);
		Direction up_right = (us == WHITE ? UP_RIGHT : DOWN_RIGHT);

		BITBOARD empty_sq = pos.get_all_empty_squares_bb();
		BITBOARD checkers = pos.get_attackers_to(pos.square<KING>(pos.side_to_move())) & pos.get_opponent_pieces_bb();

		BITBOARD enemies = type == GT_EVASION ? checkers : pos.get_pieces_bb(them);

		BITBOARD pawn_on_7th = pos.get_pieces_bb(PAWN, us) & T7th_bb;
		BITBOARD pawn_not_on_7th = pos.get_pieces_bb(PAWN, us) & ~T7th_bb;

		// single and double pawn pushes, without promotions
		if (type != GT_CAPTURE)
		{
			BITBOARD b1 = move_to<up>(pawn_on_7th) & empty_sq;
			BITBOARD b2 = move_to<up>(b1 & T3rd_bb) & empty_sq;
		}

		// Promotions and underpromotions

		// Standart and en passant captures
	}
}