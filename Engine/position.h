﻿#ifndef POSITION_H
#define POSITION_H

#include "defs.h"
#include <map>


// Starting fen string
static const char* START_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 ";

// Some example fen string
static const char* TEST_FEN = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 ";
static const char* TEST_ATTACKS_FEN = "8/8/8/3N4/8/8/8/8 w - - ";

// side to move
int side;

// en passant square
int enpassant = NONE;

// castling bit
int castle;

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

char ascii_pieces[13] = "PNBRQKpnbrqk";

//static char tricky_position[] = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 ";

// char unicode_pieces[12] = { '♙','♘','♗','♖','♕','♔','♟','♞','♝','♜','♛','♚' };

// piece bitboards
static U64 bitboards[12];
static U64 occupancies[3];

namespace Position
{
	void init(const char* fen);
}


// fen string input output
extern inline void set(const char* fenStr);

// Board representation
extern inline void print_board();

// Helper methods
extern inline Piece get_piece(const char& symbol);


#endif // !POSITION_H
