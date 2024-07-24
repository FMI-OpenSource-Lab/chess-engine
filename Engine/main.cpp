#include "bitboard.h"
#include "attacks.h"

#include <iostream>
#include <map>

// piece bitboards
U64 bitboards[12];

// occuancy bitboards
U64 occupancies[3];

// side to move
int side = -1;

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
}

void init_all()
{
	initAttacks();

	Bitboards::init();
}

void load_fen(std::string fen)
{
	std::map<char, int> pieceTypeFromSymbolMp;

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
	int rank = 7;

	// Not tested
	for (int i = 0; i < fen.length(); i++)
	{
		if (fen[i] == '/')
		{
			file = 0;
			rank--;
		}
		else
		{
			if (isdigit(fen[i]))
				file += ('0' + fen[i]);
			else
			{
				Color color = isupper(fen[i]) ? WHITE : BLACK;
				PieceType py = static_cast<PieceType>(pieceTypeFromSymbolMp[(char)tolower(fen[i])]);
				bitboards[rank * 8 + file] = py | color;
				file++;
			}
		}
	}
}

void set_up_pieces()
{
	// init pawns
	for (Square s = A2; s <= H2; ++s)
	{
		set_bit(bitboards[WHITE_PAWN], s);
		set_bit(bitboards[BLACK_PAWN], s - (8 * 5));
	}

	// init heavy pieces
	for (Square s = A1; s <= H1; ++s)
	{
		if (s % 8)

			set_bit(bitboards[WHITE_ROOK], A1);
		set_bit(bitboards[WHITE_ROOK], H1);

		set_bit(bitboards[BLACK_ROOK], A8);
		set_bit(bitboards[BLACK_ROOK], H8);
	}

}

int main()
{
	init_all();

	set_up_pieces();

	print_board();

	system("pause");
	return 0;
}