#ifndef MOVE_H
#define MOVE_H

#include <cstdint>
#include <string.h>

#include "defs.h"
#include "bitboard.h"

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

	enum MoveType : uint16_t
	{
		MT_NORMAL,
		MT_ONLY_CAPTURES
	};

	enum PromotionType : uint8_t
	{
		PROMOTION_KNIGHT,
		PROMOTION_BISHOP,
		PROMOTION_ROOK,
		PROMOTION_QUEEN,
	};

	typedef struct
	{
		// moves
		int moves[256];

		// move count
		int count;
	} moves;

	extern inline void generate_moves(moves* move_list);

	extern inline void pawn_moves(moves* move_list);
	extern inline void castle_moves(moves* move_list);
	extern inline void piece_moves(PieceType pt, moves* move_list);
	extern int make_move(int move, int move_flag);

	// Get all pawn attacks on the respective bits on the board
	inline U64 all_board_pawn_attacks(U64 wattacks[], U64 wpawns)
	{
		U64 pawn_attack_bb = 0ULL;

		while (wpawns)
		{
			int source = getLS1B(wpawns);
			pawn_attack_bb |= wattacks[source] | wpawns;
			resetLSB(wpawns);
		}

		return pawn_attack_bb;
	}

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

	extern void print_move(int move);
	extern inline void print_move_list(moves* move_list);
	extern inline void add_move(moves* move_list, int move);
}
#endif // !MOVE_H
