#include "perft.h"

namespace ChessEngine
{
	inline std::string move(Move m)
	{
		if (m == Move::invalid_move())
			return "(none)";

		if (m == Move::null_move())
			return "0000";

		Square source = m.source_square();
		Square target = m.target_square();

		if (m.move_type() == MT_CASTLING)
			target = make_square(target > source ? FILE_G : FILE_C, rank_of(source));

		std::string move = squareToCoordinates[source];
		move += squareToCoordinates[target];

		if (m.move_type() == MT_PROMOTION)
			move += tolower(ascii_pieces[m.promoted()]);

		return move;
	}

}