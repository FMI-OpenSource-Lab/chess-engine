#ifndef POSITION_H
#define POSITION_H

#include "defs.h"

#include <vector>

namespace ChessEngine
{
	// Empty fen string
	static const char* EMPTY_FEN = "8/8/8/8/8/8/8/8 b - - ";

	// Starting fen string
	static const char* START_FEN = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ";

	// Some example fen string
	static const char* TEST_FEN = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 ";
	static const char* TEST_ATTACKS_FEN = "8/8/8/3PN3/8/8/3p4/8 w - - ";

	// side to move
	extern Color side;

	// en passant square
	extern Square enpassant;

	/*
		binary representation of castling rights

		bin		dec
		0001	1 white king can castle to the king side
		0010	2 white king can castle to the queen gside
		0100	4 black king can castle to the king side
		1000	8 black king can castle to the queen side
	*/

	// examples
	/*	1111 both can castle both directions
		1001	white king => king side
				black king => queen side
	*/

	// castling bit
	extern CastlingRigths castle;

	static char ascii_pieces[13] = "PNBRQKpnbrqk";

	//static char tricky_position[] = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 ";

	// char unicode_pieces[12] = { '♙','♘','♗','♖','♕','♔','♟','♞','♝','♜','♛','♚' };

	// piece bitboards
	extern U64 bitboards[12];
	extern U64 occupancies[3];

	namespace Position
	{
		void init(const char* fen);
	}

	// fen string input output
	void set(const char* fen);

	// Board representation
	extern inline void print_board();

	// Helper methods
	inline Piece get_piece(const char& symbol);
}
#endif // !POSITION_H
