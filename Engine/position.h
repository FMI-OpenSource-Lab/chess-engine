#ifndef POSITION_H
#define POSITION_H

#include <map>

#include "defs.h"

namespace ChessEngine
{


	// Map that stores pieces as symbols
	static std::map<char, Piece> pieceTypeFromSymbolMp;

	// Starting fen string
	static const char* START_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 ";

	// Some example fen string
	static const char* TEST_FEN = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 ";
	static const char* TEST_ATTACKS_FEN = "8/8/8/3P4/8/8/8/8 w - - ";

	// side to move
	static Color side;

	// en passant square
	static unsigned short enpassant = NONE;

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
	static short castle;

	static char ascii_pieces[13] = "PNBRQKpnbrqk";

	//static char tricky_position[] = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 ";

	// char unicode_pieces[12] = { '♙','♘','♗','♖','♕','♔','♟','♞','♝','♜','♛','♚' };

	// piece bitboards
	static U64 bitboards[12];
	static U64 occupancies[3];

	namespace Position
	{
		void init(const char* fen, Color& c);
	}

	// print attacked squares
	extern void print_attacked_squares(Color& colour);

	// fen string input output
	void set(const char* fen);

	// Board representation
	extern inline void print_board();

	// Helper methods
	void init_pieces();
	// inline bool is_attacked(const Square& square, const Color& side_to_move);
	inline Piece get_piece(const char& symbol);

}
#endif // !POSITION_H
