#pragma once

#include "defs.h"
#include "position.h"

namespace KhaosChess
{
    class Position;

	// This namespace aims to improve the evaluation of the position
	// by using a combination of material and positional evaluation
	// The evaluation is done using a combination of material and positional evaluation
	namespace Eval
	{
		Value simple_evaluation(const Position &);
		Value evaluate(const Position &);
	} // namespace Eval
} // namespace KhaosChess