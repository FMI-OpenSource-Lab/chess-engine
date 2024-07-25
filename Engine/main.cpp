#include "bitboard.h"
#include "attacks.h"

#include <iostream>
#include <map>

// piece bitboards
U64 bitboards[12];

// occuancy bitboards
U64 occupancies[3];

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

char unicode_pieces[12] = { '♙','♘','♗','♖','♕','♔','♟','♞','♝','♜','♛','♚' };

void print_board()
{
	std::cout << "\n";

	// loop on ranks
	for (Rank rank = RANK_1; rank <= RANK_8; ++rank)
	{
		// loop on files
		for (File file = FILE_A; file <= FILE_H; ++file)
		{
			int squareIndex = rank * 8 + file;

			if (!file) printf("  %d ", 8 - rank);

			int pieceIndex = -1;

			// loop over all the pieces bitboards
			for (Piece piece_bb = WHITE_PAWN; piece_bb <= BLACK_KING; ++piece_bb)
			{
				if (get_bit(bitboards[piece_bb], squareIndex))
					pieceIndex = piece_bb;
			}

			printf(" %c", (pieceIndex == -1) ? '.' : ascii_pieces[pieceIndex]);
		}

		std::cout << std::endl;
	}

	// print files
	std::cout << "\n     a b c d e f g h \n\n";

	printf("	Side:		%s\n", (!side) ? "white" : "black");
	printf("	Enpassant:	%s\n", (enpassant != NONE) ? squareToCoordinates[enpassant] : "no");
	printf("	Castling:	%c%c%c%c\n",
		(castle & WK) ? 'K' : '-',
		(castle & WQ) ? 'Q' : '-',
		(castle & BK) ? 'k' : '-',
		(castle & BQ) ? 'q' : '-');
}

void parse_fen(char* fen)
{


	// loop over board squares
	for (Rank rank = RANK_1; rank <= RANK_8; ++rank)
	{
		for (File file = FILE_A; file <= FILE_H; ++file)
		{
			if ((*fen >= 'a' && *fen <= 'z'))
			{

			}
		}
	}

}

void load_fen(char* fenPtr)
{
	// reset boards and state variables
	memset(bitboards, 0ULL, sizeof(bitboards));
	memset(occupancies, 0ULL, sizeof(occupancies));

	side = 0;
	enpassant = NONE;
	castle = 0;

	std::map<char, Piece> pieceTypeFromSymbolMp;

	pieceTypeFromSymbolMp['p'] = BLACK_PAWN;
	pieceTypeFromSymbolMp['n'] = BLACK_KNIGHT;
	pieceTypeFromSymbolMp['b'] = BLACK_BISHOP;
	pieceTypeFromSymbolMp['r'] = BLACK_ROOK;
	pieceTypeFromSymbolMp['q'] = BLACK_QUEEN;
	pieceTypeFromSymbolMp['k'] = BLACK_KING;

	pieceTypeFromSymbolMp['P'] = WHITE_PAWN;
	pieceTypeFromSymbolMp['N'] = WHITE_KNIGHT;
	pieceTypeFromSymbolMp['B'] = WHITE_BISHOP;
	pieceTypeFromSymbolMp['R'] = WHITE_ROOK;
	pieceTypeFromSymbolMp['Q'] = WHITE_QUEEN;
	pieceTypeFromSymbolMp['K'] = WHITE_KING;

	int file = 0;
	int rank = 0;

	// loop over char elements
	for (char c = *fenPtr; c != ' '; c = *++fenPtr)
	{
		if (c == '/') // new line
		{
			// resent file
			// go to the next rank
			file = 0;
			rank++;
		}
		else
		{
			if (c >= '0' && c <= '9')
			{
				// leve this many empty spaces
				// c derefrences the pointer and subtracs '0' to convert it into int
				file += c - '0';
			}
			else // its a piece
			{
				Square sq = static_cast<Square>(8 * rank + file);
				set_bit(bitboards[pieceTypeFromSymbolMp[c]], sq);
				file++;
			}
		}
	}

	// for paring side to move
	// increment to reach the side to move
	*fenPtr++;

	// side to move
	(*fenPtr == 'w') ? (side = WHITE) : (side = BLACK);

	// go to castling rights
	fenPtr += 2;

	// parse castling rights
	while (*fenPtr != ' ')
	{
		switch (*fenPtr)
		{
		case 'K': castle |= WK; break;
		case 'Q': castle |= WQ; break;
		case 'k': castle |= BK; break;
		case 'q': castle |= BQ; break;
		case '-': break;
		}

		*fenPtr++;
	}

	*fenPtr++;

	// enpassant square
	if (*fenPtr != '-')
	{
		// parse file and rank
		int file = fenPtr[0] - 'a';
		int rank = 8 - (fenPtr[1] - '0');

		// init enpassant suqare
		enpassant = rank * 8 + file;
	}
	else
		enpassant = NONE;

	// populate occupancy bitboard
	for (Piece piece = WHITE_PAWN; piece <= WHITE_KING; ++piece)
	{
		occupancies[WHITE] |= bitboards[piece];
		occupancies[BLACK] |= bitboards[piece + 6]; // + 6 pieces
	}
}

void init_all()
{
	initAttacks();

	Bitboards::init();

	// init side to move
	side = WHITE;
	// set enpassant square
	enpassant = E3;
	// init castling rights
	castle |= WK | WQ | BK | BQ;
	char tricky_position[] = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 ";
	// load the starting fen
	load_fen(tricky_position);
}

int main()
{
	init_all();

	print_board();
	
	print_bitboard(occupancies[WHITE]);
	print_bitboard(occupancies[BLACK]);

	system("pause");
	return 0;
}