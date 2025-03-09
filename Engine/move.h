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
		0011 0000 0000 0000 move type					| 1 bit
		1100 0000 0000 0000 promotion type				| 2 bits

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
		MT_NORMAL = 0,
		MT_PROMOTION,
		MT_EN_PASSANT,
		MT_CASTLING
	};

	enum PromotionType
	{
		PROMOTE_KNIGHT = 0,
		PROMOTE_BISHOP,
		PROMOTE_ROOK,
		PROMOTE_QUEEN,
	};

	class Move
	{
	public:
		inline Move() : move(0) {};

		constexpr explicit Move(std::uint16_t m) : move(m) {}

		constexpr Move(Square source, Square target) : move((target << 6) + source) {}

		constexpr Move(Square source, Square target, MoveType mt) : move((mt << 12) + (target << 6) + source) {}

		constexpr Move(Square source, Square target, MoveType mt, PromotionType pt) : move((pt << 14) + (mt << 12) + (target << 6) + source) {}

		constexpr Square source_square() const { return Square(move & 0x3F); }
		constexpr Square target_square() const { return Square((move >> 6) & 0x3F); }

		constexpr MoveType move_type() const { return MoveType((move >> 12) & 3); }
		constexpr PieceType promoted() const { return PieceType(((move >> 14) & 3) + KNIGHT); }

		// null and none moves
		// has the same source and target square whilst other moves have different source and destination squares
		static constexpr Move null_move() { return Move(65); }
		static constexpr Move invalid_move() { return Move(0); }

		constexpr std::uint16_t move_value() const { return move; }
		constexpr bool is_move_ok() const { return invalid_move().move != move && null_move().move != move; }

		static constexpr bool same_move(Move a, Move b) { return a.move == b.move; }

		friend std::ostream& operator<<(std::ostream& os, Move const& mv)
		{
			os << squareToCoordinates[mv.source_square()] << squareToCoordinates[mv.target_square()];

			if (mv.move_type() == MT_PROMOTION)
				os << " PNBRQKpnbrqk"[get_piece(BLACK, mv.promoted())]; // BLACK because we want the lowercase letter of the promtoed piece

			return os;
		}

        std::string uci_move() const
		{
			if (*this == invalid_move())	return "(none)";
			if (*this == null_move())		return "0000";

			Square source = source_square();
			Square target = target_square();

			if (move_type() == MT_CASTLING)
				target = make_square(target > source ? FILE_G : FILE_C, rank_of(source));

			std::string move_str = squareToCoordinates[source];
			move_str += squareToCoordinates[target];

			if (move_type() == MT_PROMOTION)
				move_str += " PNBRQKpnbrqk"[get_piece(BLACK, promoted())];

			return move_str;
		}

		// overloads
		constexpr bool operator==(const Move& m) const { return move == m.move; }
		constexpr bool operator!=(const Move& m) const { return move != m.move; }
		constexpr explicit operator bool() const { return move != 0; }
	protected:
		std::uint16_t move;
	};

	const Move NO_MOVE = Move();
}
#endif // !MOVE_H
