#pragma once
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

	typedef struct
	{
		// moves
		int moves[256];

		// move count
		int count;
	} moves;


	template<GenerationTypes>
	Move* generate_moves(const Position& pos, Move* move_list);

	template<GenerationTypes T>
	struct MoveList {
		explicit MoveList(const Position& pos) :
			last(generate_moves<T>(pos, move_list)) {}

		const Move* begin() const { return move_list; }
		const Move* end() const { return last; }
		size_t		size() const { return last - move_list; }
		bool		contains(Move m) const { return std::find(begin(), end(), m) != end(); }

	private:
		Move move_list[MAX_MOVES], * last;
	};

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
