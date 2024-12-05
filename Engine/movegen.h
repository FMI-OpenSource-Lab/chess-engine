#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "defs.h"
#include "move.h"
#include "position.h"

#include<cstdint>

namespace ChessEngine
{
	class Position;

	enum GenerationTypes
	{
		GT_CAPTURE,
		GT_QUIET,
		GT_EVASION,
		GT_NON_EVATION,
		GT_LEGAL
	};

	// TODO:
	// When generating pawn moves an alternative approach will be to generate the
	// promotions on different function, then everything else
	// as well as adding more classifications for the moves such as
	// CAPTURE, QSEARCH, QUIETS and etc.

	struct ScoredMove : public Move
	{
		int score = 0;
		void operator=(Move m) { move = m.move_value(); }
	};

	template<GenerationTypes>
	ScoredMove* generate_moves(const Position& pos, ScoredMove* move_list);
}
#endif // !MOVEGEN_H
