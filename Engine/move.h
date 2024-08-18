#ifndef MOVE_H
#define MOVE_H

#include <cstdint>

#include "defs.h"

namespace ChessEngine
{
	// A move needs 16 bits to be stored
	//
	// bit  0-5: destination square (from 0 to 63)
	// bit  6-11: source square (from 0 to 63)
	// bit 12-13: promotion piece type - 2 (from KNIGHT-2 to QUEEN-2)
	// bit 14-15: special move flag: promotion (1), en passant (2), castling (3)

	enum MoveType : uint16_t
	{
		NORMAL,
		PROMOTION = 1 << 14,
		EN_PASSANT = 2 << 14,
		CASTLING = 3 << 14
	};

	enum PromotionType : uint8_t
	{
		PROMOTION_KNIGHT,
		PROMOTION_BISHOP,
		PROMOTION_ROOK,
		PROMOTION_QUEEN,
	};

	class Move
	{
	private:
		// Bits are arranged
		// 2 bits for promotion piece | 2 bits for type | 6 bits for from | 6 bits for to
		uint16_t move;
	public:
		Move() = default;
		constexpr explicit Move(std::uint16_t m) : move(m) {}

		constexpr Move(Square source, Square target)
			: move((source << 6) + target) {}
	};

	extern inline void generate_moves();
}
#endif // !MOVE_H
