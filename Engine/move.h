#ifndef MOVE_H
#define MOVE_H

#include <cstdint>
#include <string.h>
#include <assert.h>
#include <ostream>

#include "defs.h"
#include "bitboard.h"

namespace ChessEngine
{
	/*
		Binary move bits

		0000 0000 0011 1111 source square				| 6 bits
		0000 1111 1100 0000 target square				| 6 bits
		0011 0000 0000 0000 promoted piece				| 2 bits
		0100 0000 0000 0000 promotion flag				| 1 bits
		1000 0000 0000 0000 enpassant capture flag		| 1 bit
		1100 0000 0000 0000 castling flag				| 2 bit

		Promoted piece types bits:

		00 -> 0: KNIGHT
		01 -> 1: BISHOP
		10 -> 2: ROOK
		11 -> 3: QUEEN

		Flags:

		01 -> 1: promotion
		10 -> 2: en passant (set only when pawn can be captured)
		11 -> 3: castling

		A move needs 16 bits to be stored
	*/

	enum MoveType
	{
		MT_NORMAL,
		MT_PROMOTION = 1 << 14,
		MT_EN_PASSANT = 2 << 14,
		MT_CASTLING = 3 << 14
	};

	enum PromotionType
	{
		PROMOTE_KNIGHT,
		PROMOTE_BISHOP = 1 << 12,
		PROMOTE_ROOK = 2 << 12,
		PROMOTE_QUEEN = 3 << 12,
	};

	class Move
	{
	public:
		Move() = default;

		constexpr explicit Move(std::uint16_t m) :
			move(m) {}

		constexpr Move(Square source, Square target) :
			move((target << 6) + source) {}

		constexpr Move(Square source, Square target, MoveType mt)
			: move(mt + (target << 6) + source) {}

		constexpr Move(Square source, Square target, MoveType mt, PromotionType pt)
			: move(mt + pt + (target << 6) + source) {}

		constexpr Square source_square() const { return Square(move & 0x3F); }

		constexpr Square target_square() const { return Square((move >> 6) & 0x3F); }

		constexpr PieceType promoted() const { return PieceType(((move >> 12) & 3) + KNIGHT); }

		constexpr MoveType move_type() const
		{
			return MoveType(move & (0b0011 << 14));
		}

		constexpr std::uint16_t move_value() { return move; }
		constexpr bool is_move_ok() const { return invalid_move().move != move && null_move().move != move; }

		// null and none moves
		// has the same source and target square whilst other moves have different source and destination squares
		static constexpr Move null_move() { return Move(64); }
		static constexpr Move invalid_move() { return Move(0); }

		static constexpr bool same_move(Move a, Move b) { return a.move == b.move; }

		friend std::ostream& operator<<(std::ostream& os, Move const& mv) {
			os << squareToCoordinates[mv.source_square()]
				<< squareToCoordinates[mv.target_square()];

			if (mv.move_type() == MT_PROMOTION)
				os << " PNBRQKpnbrqk"[get_piece(BLACK, mv.promoted())]; // BLACK because we want the lowercase letter of the promtoed piece

			return os;
		}

		// overloads
		constexpr bool operator==(const Move& m) const { return move == m.move; }
		constexpr bool operator!=(const Move& m) const { return move != m.move; }
		constexpr explicit operator bool() const { return move != 0; }
	protected:
		std::uint16_t move = 0;
	};

	const Move NO_MOVE = Move();
}
#endif // !MOVE_H
