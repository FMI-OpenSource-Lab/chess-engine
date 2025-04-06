#pragma once

#include "movegen.h"
#include "defs.h"
#include "consts.h"
#include "position.h"

#include <array>

namespace KhaosChess
{
	constexpr int piece_attack_weight[PIECE_TYPE_NB] = {0, 1, 3, 3, 5, 9, 0};

	extern BITBOARD king_shield_mask[BOTH][SQUARE_TOTAL];
	extern BITBOARD in_front_bb[BOTH][RANK_NB];
	extern Value connected_bonus[RANK_NB];
	extern Value passed_bonus[RANK_NB];

	class Position;

	// This namespace aims to improve the evaluation of the position
	// by using a combination of material and positional evaluation
	// The evaluation is done using a combination of material and positional evaluation
	namespace Eval
	{
		void init();

		template <Color c>
		Value simple_evaluation(const Position &);

		template <Color c>
		Value mobility(const Position &);

		template <Color c>
		Value king_safety(const Position &);

		template <Color c>
		Value pawn_structure(const Position &);

		template <Color c>
		Value piece_coordination(const Position &);

		template <Color c>
		Value threats(const Position &);

		Value evaluate(const Position &);
	} // namespace Eval
} // namespace KhaosChess
