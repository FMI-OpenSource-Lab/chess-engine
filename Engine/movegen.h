#pragma once
#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "defs.h"

#include<cstdint>

namespace ChessEngine
{
	enum MoveType : int
	{
		MT_NORMAL,
		MT_ONLY_CAPTURES
	};

	typedef struct
	{
		// moves
		int moves[256];

		// move count
		int count;
	} moves;

	// Generate moves functions
	extern void generate_moves(moves* move_list);

	extern inline void pawn_moves(moves* move_list);
	extern inline void castle_moves(moves* move_list);
	extern inline void piece_moves(PieceType pt, moves* move_list);

	// Make move functions
	extern int make_move(int move, int move_flag);
	extern inline void add_move(moves* move_list, int move);

	// Prints
	extern void print_move(int move);
	extern inline void print_move_list(moves* move_list);
}
#endif // !MOVEGEN_H
