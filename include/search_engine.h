#pragma once

#include "defs.h"
#include "move.h"
#include "position.h"

#include <atomic>
#include <chrono>
#include <vector>

namespace KhaosChess
{
	class Position;
	class Move;

	extern Value negamax(Value alpha, Value beta, int depth, Position &pos);
	extern Value quiescence(Value alpha, Value beta, Position &pos);

	extern void search_position(int depth, Position &pos);
} // namespace KhaosChess
