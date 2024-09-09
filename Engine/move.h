#ifndef MOVE_H
#define MOVE_H

#include <cstdint>
#include <string.h>

#include "defs.h"
#include "bitboard.h"

#include <assert.h>

namespace ChessEngine
{
	/*
		Binary move bits

		0000 0000 0000 0000 0011 1111 source square				| 6 bits
		0000 0000 0000 1111 1100 0000 target square				| 6 bits
		0000 0000 1111 0000 0000 0000 piece						| 4 bits
		0000 1111 0000 0000 0000 0000 promoted piece			| 4 bits
		0001 0000 0000 0000 0000 0000 capture flag				| 1 bit
		0010 0000 0000 0000 0000 0000 double push flag			| 1 bit
		0100 0000 0000 0000 0000 0000 enpassant capture flag	| 1 bit
		1000 0000 0000 0000 0000 0000 castling flag				| 1 bit

		Hexidecimal constants

		source square			| 0x3f
		target square			| 0xfc0
		piece					| 0xf000
		promoted piece			| 0xf0000
		capture flag			| 0x100000
		double push flag		| 0x200000
		enpassan capture flag	| 0x400000
		castling flag			| 0x800000
	*/

	class Move
	{
	public:
		Move() = default;
		constexpr explicit Move(std::uint32_t m) :
			move(m) {}

		constexpr Move(Square target, Square source) :
			move((target << 6) + source) {}

		template<MoveType mt>
		static constexpr Move make(Square target, Square source, PieceType pt = KNIGHT)
		{
			return Move(mt + ((pt - KNIGHT) << 12) + (target << 6) + source);
		}

		constexpr Square source_square() const
		{
			assert(is_ok());
			return Square((move >> 6) & 0x3f);
		}

		constexpr Square target_square() const
		{
			assert(is_ok());
			return Square(move & 0x3F);
		}

		constexpr Piece piece() const { return Piece((move & 0xf000) >> 12); }
		constexpr Piece promoted() const { return Piece((move & 0xf0000) >> 16); }

		constexpr std::uint32_t capture_flag() const { return move & 0x100000; }
		constexpr std::uint32_t doublep_flag() const { return move & 0x200000; }
		constexpr std::uint32_t enpassant_flag() const { return move & 0x400000; }
		constexpr std::uint32_t castling_flag() const { return move & 0x800000; }

		std::uint32_t encode(Square source, Square target, Piece piece, Piece promoted, std::int32_t capturef, std::int32_t doublef, std::int32_t enpassantf, std::int32_t castlingf) {
			return (
				(source << 0) |
				(target << 6) |
				(piece << 12) |
				(promoted << 16) |
				(capturef << 20) |
				(doublef << 21) |
				(enpassantf << 22) |
				(castlingf << 23));
		}

		constexpr bool is_ok() const { return none().move != move && null().move != move; }

		constexpr std::uint32_t raw() { return move; }

		// null and none moves
		// has the same source and target square whilst other moves have different source and destination squares
		static constexpr Move null() { return Move(64); }
		static constexpr Move none() { return Move(0); }

		// overloads
		constexpr bool operator==(const Move& m) const { return move == m.move; }
		constexpr bool operator!=(const Move& m) const { return move != m.move; }
		constexpr explicit operator bool() const { return move != 0; }
	protected:
		std::uint32_t move;
	};


#define encode_move(source, target, piece, promoted, capturef, doublef, enpassantf, castlingf)	\
	(source << 0) |			\
	(target << 6) |			\
	(piece << 12) |			\
	(promoted << 16) |		\
	(capturef << 20) |		\
	(doublef << 21) |		\
	(enpassantf << 22) |	\
	(castlingf << 23)

#define get_move_source(move) (move & 0x3f)
#define get_move_target(move) ((move & 0xfc0) >> 6)
#define get_move_piece(move) ((move & 0xf000) >> 12)
#define get_move_promoted(move) ((move & 0xf0000) >> 16)
#define get_move_capture(move) (move & 0x100000)
#define get_move_double(move) (move & 0x200000)
#define get_move_enpassant(move) (move & 0x400000)
#define get_move_castling(move) (move & 0x800000)

	// 96 -> sizeof(bitboards)
	// 24 -> sizeof(occuapncies)

#define copy_board() \
	U64 bitboards_copy[12], occupancies_copy[3];	\
		Color side_copy;							\
		Square enpassant_copy;						\
		CastlingRigths castle_copy;					\
		memcpy(bitboards_copy, bitboards, 96);		\
		memcpy(occupancies_copy, occupancies, 24);	\
		side_copy = side,							\
		enpassant_copy = enpassant,					\
		castle_copy = castle;						\

#define restore_board() \
		memcpy(bitboards, bitboards_copy, 96);		\
		memcpy(occupancies, occupancies_copy, 24);  \
		side = side_copy,							\
		enpassant = enpassant_copy,					\
		castle = castle_copy;						\

	// Get all pawn attacks on the respective bits on the board
	inline U64 all_board_pawn_attacks(U64 attacks[], U64 pawns)
	{
		U64 pawn_attack_bb = 0ULL;

		while (pawns)
		{
			int source = getLS1B(pawns);

			pawn_attack_bb |= attacks[source];
			resetLSB(pawns);
		}

		return pawn_attack_bb;
	}

	// go nort
	constexpr U64 down_one(U64 b) { return b << 8; }
	// go sout
	constexpr U64 up_one(U64 b) { return b >> 8; }

	// Determine pawn push target squares or their stop squares set-wise
	// To generate the single-step targets for all pawns
	// requires vertical shift by one rank and intersection with the set of empty squares.
	inline U64 white_single_push_target(U64 wpawns, U64 empty) { return up_one(wpawns) & empty; }

	inline U64 black_single_push_target(U64 bpawns, U64 empty) { return down_one(bpawns) & empty; }

	inline U64 white_double_push_target(U64 wpawns, U64 empty)
	{
		U64 singlePushs = white_single_push_target(wpawns, empty);
		return up_one(singlePushs) & empty & Rank4_Bits;
	}

	inline U64 black_double_push_target(U64 bpawns, U64 empty)
	{
		U64 singlePushs = black_single_push_target(bpawns, empty);
		return down_one(singlePushs) & empty & Rank5_Bits;
	}
}
#endif // !MOVE_H
