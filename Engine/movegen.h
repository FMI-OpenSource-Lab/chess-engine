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

}
#endif // !MOVEGEN_H
