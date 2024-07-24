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

enum { WK = 1, WQ = 2, BK = 4, BQ = 8 };

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

char start_fen[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR";

void load_fen(char* fenPtr)
{
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
	for (char c = *fenPtr; c; c = *++fenPtr)
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
			if (isdigit(c))
			{
				// leve this many empty spaces
				// probably mod 8 empty spaces

				file += ('0' + c);
			}
			else // its a piece
			{
				Square sq = static_cast<Square>(8 * rank + file);
				set_bit(bitboards[pieceTypeFromSymbolMp[c]], sq);
				file++;
			}
		}
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

	// load the starting fen
	load_fen(start_fen);
}

int main()
{
	init_all();

	print_board();

	system("pause");
	return 0;
}