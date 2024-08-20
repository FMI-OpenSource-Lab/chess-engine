#ifndef MOVE_H
#define MOVE_H

#include <cstdint>

#include "defs.h"
#include "bitboard.h"

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
		MT_NORMAL,
		MT_EN_PASSANT,
		MT_CASTLING,
		MT_PROMOTION
	};

	enum PromotionType : uint8_t
	{
		PROMOTION_KNIGHT,
		PROMOTION_BISHOP,
		PROMOTION_ROOK,
		PROMOTION_QUEEN,
	};

	// go nort
	inline U64 down_one(U64 b) { return b << 8; }
	// go sout
	inline U64 up_one(U64 b) { return b >> 8; }

	// Determine pawn push target squares or their stop squares set-wise
	// To generate the single-step targets for all pawns
	// requires vertical shift by one rank and intersection with the set of empty squares.
	inline U64 white_single_push_target(U64 wpawns, U64 empty) { return up_one(wpawns) & empty; }
	inline U64 white_p_able_to_push(U64 wpawns, U64 empty) { return up_one(empty) & wpawns; }

	inline U64 black_single_push_target(U64 bpawns, U64 empty) { return down_one(bpawns) & empty; }
	inline U64 black_p_able_to_push(U64 bpawns, U64 empty) { return down_one(empty) & bpawns; }

	inline U64 white_double_push_target(U64 wpawns, U64 empty)
	{
		U64 singlePushs = white_single_push_target(wpawns, empty);
		return up_one(singlePushs) & empty & Rank4_Bits;
	}

	inline U64 white_p_able_to_double_push(U64 wpawns, U64 empty)
	{
		U64 emptyRank3 = up_one(empty & Rank4_Bits) & empty;
		return white_p_able_to_push(wpawns, emptyRank3);
	}

	inline U64 black_double_push_target(U64 bpawns, U64 empty)
	{
		U64 singlePushs = black_single_push_target(bpawns, empty);
		return down_one(singlePushs) & empty & Rank5_Bits;
	}

	inline U64 black_p_able_to_double_push(U64 bpawns, U64 empty)
	{
		U64 emptyRank6 = down_one(empty & Rank5_Bits) & empty;
		return black_p_able_to_push(bpawns, emptyRank6);
	}

	extern inline void generate_moves();

	class Move
	{
	private:
		// How bits are arranged
		// promotion piece - 2 bits
		// type - 2 bits 
		// source - 6 bits
		// target - 6 bits
		uint16_t move;
	public:
		Move() = default;
		constexpr explicit Move(std::uint16_t m) : move(m) {}

		constexpr Move(Square source, Square target)
			: move((source << 6) + target) {}

		inline Move(Square source, Square target, MoveType mt)
			: move((mt << 12) | (source << 6) | (target << 0)) { }

		constexpr Square from_square() const
		{
			return Square((move >> 6) & 0x3F);
		}

		constexpr Square to_square() const
		{
			return Square(move & 0x3F);
		}

		constexpr Square target() const { return Square(move & 0x3F); }
		constexpr Square source() const { return Square((move >> 6) & 0x3F); }
		constexpr MoveType type() const { return MoveType((move >> 12) & 0x3); }
		constexpr PromotionType promotion_type() const { return PromotionType((move >> 14) & 0x3); }

		bool operator==(Move m) const { return move == m.move; }
		bool operator!=(Move m) const { return move != m.move; }
	};
}

#endif // !MOVE_H
