#pragma once

#include "movegen.h"
#include "defs.h"
#include "consts.h"
#include "position.h"

#include <array>

namespace KhaosChess
{
	struct EvalInfo
	{
		int game_phase = 0; // Game phase
		int passed_pawn_count = 0;
		int piece_counts[BOTH][PIECE_TYPE_NB]{};

		int total_king_ring_attacks[BOTH]{};
	
		Square king_squares[2]{};
	
		BITBOARD pawns[2]{};
		BITBOARD pieces[2]{};
		BITBOARD pawn_attacks[2]{};
	
		BITBOARD piece_relative_occupancies[2][6]{};
	};

	class Position;

	// This namespace aims to improve the evaluation of the position
	// by using a combination of material and positional evaluation
	// The evaluation is done using a combination of material and positional evaluation
	namespace Eval
	{
		void init_eval_info(EvalInfo &ei, const Position &pos);

		template <Color c>
		Value simple_evaluation(const Position &);

		Value king_pawn(Color c, File file, EvalInfo &ei);

		Value evaluate(const Position &);
	} // namespace Eval
} // namespace KhaosChess
