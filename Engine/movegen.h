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
		GT_CAPTURE,		// Capturing a piece
		GT_QUIET,		// No captures nor promotions
		GT_EVASION,		// Evade (escape or block) a check
		GT_NOISY,		// Combination of capture and evasion
		GT_LEGAL		// Only legal moves
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

	template<GenerationTypes T>
	struct MoveList
	{
		explicit MoveList(const Position& pos) : last(generate_moves<T>(pos, move_list)) {}

		const ScoredMove* begin() const { return move_list; }
		const ScoredMove* end() const { return last; }
		size_t size() const { return last - move_list; }

		bool contains_move(Move m) const { return std::find(begin(), end(), m) != end(); }
	private:
		ScoredMove move_list[MAX_MOVES], * last;
	};
}
#endif // !MOVEGEN_H
