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
		GT_EVADE,		// Evade (escape or block) a check
		GT_ALL,			// Quiet and capture moves
		GT_LEGAL		// Only legal moves
	};

	struct ScoredMoves : public Move
	{
		int score = 0;
		void operator=(Move m) { move = m.move_value(); }
	};

	template<GenerationTypes>
	inline ScoredMoves* generate_moves(const Position& pos, ScoredMoves* move_list);

	template<GenerationTypes T>
	struct MoveList
	{
		explicit MoveList(const Position& pos) : last(generate_moves<T>(pos, moves_list)) {}

		const ScoredMoves* begin() const { return moves_list; }
		const ScoredMoves* end() const { return last; }
		size_t			   size() const { return last - moves_list; }
		bool			   contains_move(Move m) const { return std::find(begin(), end(), m) != end(); }
	private:
		ScoredMoves moves_list[MAX_MOVES], * last;
	};
}
#endif // !MOVEGEN_H
