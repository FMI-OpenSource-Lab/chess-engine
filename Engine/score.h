#ifndef SCORE_H
#define SCORE_H

#include "movegen.h"
#include "defs.h"

#include<array>

namespace ChessEngine
{
	/*
		Material score

		P = 100,	= P
		N = 300,	= P * 3
		B = 350,	= P * 3 + P * 0.5
		R = 500,	= P * 5
		Q = 1000,	= P * 10
		K = 10000	= P * 100
	*/

	static constexpr std::array<Value, 12>MATERIAL_SCORE = {
		100,	// white pawn score
		300,	// white knight score
		350,	// white bishop score
		500,	// white rook score
		1000,	// white queen score
		10000,	// white king score
		-100,	// black pawn score
		-300,	// black knight score
		-350,	// black bishop score
		-500,	// black rook score
		-1000,	// black queen score
		-10000	// black king score
	};

	// pawn positional score
	static constexpr std::array<Value, 64>PAWN_SCORE =
	{
		90,  90,  90,  90,  90,  90,  90,  90,
		30,  30,  30,  40,  40,  30,  30,  30,
		20,  20,  20,  30,  30,  30,  20,  20,
		10,  10,  10,  20,  20,  10,  10,  10,
		 5,   5,  10,  20,  20,   5,   5,   5,
		 0,   0,   0,   5,   5,   0,   0,   0,
		 0,   0,   0, -10, -10,   0,   0,   0,
		 0,   0,   0,   0,   0,   0,   0,   0
	};

	// knight positional score
	static constexpr std::array<Value, 64>KNIGHT_SCORE =
	{
		-5,   0,   0,   0,   0,   0,   0,  -5,
		-5,   0,   0,  10,  10,   0,   0,  -5,
		-5,   5,  20,  20,  20,  20,   5,  -5,
		-5,  10,  20,  30,  30,  20,  10,  -5,
		-5,  10,  20,  30,  30,  20,  10,  -5,
		-5,   5,  20,  10,  10,  20,   5,  -5,
		-5,   0,   0,   0,   0,   0,   0,  -5,
		-5, -10,   0,   0,   0,   0, -10,  -5
	};

	// bishop positional score
	static constexpr std::array<Value, 64>BISHOP_SCORE =
	{
		 0,   0,   0,   0,   0,   0,   0,   0,
		 0,   0,   0,   0,   0,   0,   0,   0,
		 0,   0,   0,  10,  10,   0,   0,   0,
		 0,   0,  10,  20,  20,  10,   0,   0,
		 0,   0,  10,  20,  20,  10,   0,   0,
		 0,  10,   0,   0,   0,   0,  10,   0,
		 0,  30,   0,   0,   0,   0,  30,   0,
		 0,   0, -10,   0,   0, -10,   0,   0

	};

	// rook positional score
	static constexpr std::array<Value, 64>ROOK_SCORE =
	{
		50,  50,  50,  50,  50,  50,  50,  50,
		50,  50,  50,  50,  50,  50,  50,  50,
		 0,   0,  10,  20,  20,  10,   0,   0,
		 0,   0,  10,  20,  20,  10,   0,   0,
		 0,   0,  10,  20,  20,  10,   0,   0,
		 0,   0,  10,  20,  20,  10,   0,   0,
		 0,   0,  10,  20,  20,  10,   0,   0,
		 0,   0,   0,  20,  20,   0,   0,   0

	};

	// king positional score
	static constexpr std::array<Value, 64>KING_SCORE =
	{
		 0,   0,   0,   0,   0,   0,   0,   0,
		 0,   0,   5,   5,   5,   5,   0,   0,
		 0,   5,   5,  10,  10,   5,   5,   0,
		 0,   5,  10,  20,  20,  10,   5,   0,
		 0,   5,  10,  20,  20,  10,   5,   0,
		 0,   0,   5,  10,  10,   5,   0,   0,
		 0,   5,   5,  -5,  -5,   0,   5,   0,
		 0,   0,   5,   0, -15,   0,  10,   0
	};

	// mirror positional score tables for opposite side
	static constexpr std::array<Value, 128>MIRROR_SCORE =
	{
		A1, B1, C1, D1, E1, F1, G1, H1,
		A2, B2, C2, D2, E2, F2, G2, H2,
		A3, B3, C3, D3, E3, F3, G3, H3,
		A4, B4, C4, D4, E4, F4, G4, H4,
		A5, B5, C5, D5, E5, F5, G5, H5,
		A6, B6, C6, D6, E6, F6, G6, H6,
		A7, B7, C7, D7, E7, F7, G7, H7,
		A8, B8, C8, D8, E8, F8, G8, H8
	};

	/*
		Most valuable victim & less valuable attacker

		(Victims)	 Pawn Knight Bishop   Rook  Queen   King
		(Attackers)

		Pawn		 105    205    305    405    505    605
		Knight		 104    204    304    404    504    604
		Bishop		 103    203    303    403    503    603
		Rook		 102    202    302    402    502    602
		Queen		 101    201    301    401    501    601
		King		 100    200    300    400    500    600

	*/

	// MVV LVA [attacker][victim]
	static constexpr Value MVV_LVA[12][12]
	{
		105, 205, 305, 405, 505, 605,  105, 205, 305, 405, 505, 605,
		104, 204, 304, 404, 504, 604,  104, 204, 304, 404, 504, 604,
		103, 203, 303, 403, 503, 603,  103, 203, 303, 403, 503, 603,
		102, 202, 302, 402, 502, 602,  102, 202, 302, 402, 502, 602,
		101, 201, 301, 401, 501, 601,  101, 201, 301, 401, 501, 601,
		100, 200, 300, 400, 500, 600,  100, 200, 300, 400, 500, 600,

		105, 205, 305, 405, 505, 605,  105, 205, 305, 405, 505, 605,
		104, 204, 304, 404, 504, 604,  104, 204, 304, 404, 504, 604,
		103, 203, 303, 403, 503, 603,  103, 203, 303, 403, 503, 603,
		102, 202, 302, 402, 502, 602,  102, 202, 302, 402, 502, 602,
		101, 201, 301, 401, 501, 601,  101, 201, 301, 401, 501, 601,
		100, 200, 300, 400, 500, 600,  100, 200, 300, 400, 500, 600
	};

	//extern int evaluate();
	//extern int score_move(int move);
	//extern int sort_move(moves* move_list);

	//extern void print_move_scores(moves* move_list);
}

#endif // !SCORE_H
