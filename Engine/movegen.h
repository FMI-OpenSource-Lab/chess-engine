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

	struct Moves : public Move
	{
		Move moves[MAX_MOVES];
		int count = 0;
		void operator=(Move m) { move = m.move_value(); }
	};

	Moves* generate_moves(const Position& pos, Moves* move_list);

	struct MoveList
	{
		explicit MoveList(Position& pos) : last(generate_moves(pos, moves_list)) {}

		const Moves* begin() const { return moves_list; }
		const Moves* end() const { return last; }
		const size_t size() const { return moves_list->count; }

		bool contains(Move m) const { return std::find(begin(), end(), m) != end(); }

	private:
		Moves moves_list[1], * last;
	};
}
#endif // !MOVEGEN_H
