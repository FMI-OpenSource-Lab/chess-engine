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

	template<GenerationTypes>
	Move* generate(Move* move_list, const Position& pos);

	template<GenerationTypes T>
	struct MoveList {
		explicit MoveList(const Position& pos) :
			last(generate<T>(move_list, pos)) {}

		const Move* begin() const { return move_list; }
		const Move* end() const { return last; }
		size_t		size() const { return last - move_list; }
		bool		contains(Move m) const { return std::find(begin(), end(), m) != end(); }

	private:
		Move move_list[MAX_MOVES], * last;
	};
}
#endif // !MOVEGEN_H
