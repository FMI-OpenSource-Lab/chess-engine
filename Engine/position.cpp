#include "position.h"

#include<iostream>
#include <sstream>

void Position::init(const char* fen)
{
	init_pieces();
	set(fen);
	print_board();

	// Color c = WHITE;
	//print_attacked_squares(c);
}



extern inline void set(const char* fen)
{
	/*
	FEN describes a Chess position, It is one line ASCII string.

	A FEN containts 6 fields separated by space

	1. Piece placement (from white's perspective).
	Each rank is separated by every / (slash). Each rank has letters and/or numbers. Letters  or  represent the pieces,
	small caps are the Black's pieces (pnbrkq) and upper case letters (PNBRKQ) are the White's pieces.
	The numbers (from 1 to 8) show how many empty spaces need to be empty

	2. Active colour or side to move. Represented as 'w' or 'b'

	3. Castling ability. If neither side can castle it is '-'.
	Otherwise FEN string indicates the castling rights using the letter 'KQ' and their representive
	lowercase kq. Uppercase letters mean castling rights for White and Lowercase letters
	mean castling rights for Black. 'K' stands for kingside and 'Q' stands for queenside


	4. En passant target square. If there's no en passant target square then the notation for this
	is simplpy '-'. Otherwise the notation is the name of the square.
	If a pawn just made a 2 square move, this is the position "behind" the pawn.

	5. Halfmove clock. This is the number of halfmoves since the last pawn advance or capture.
	This is used to determine if a draw can be claimed under the 50-move rule.

	6. Fullmove number. The number of the full move.
	It starts at 1 and is incremented after Black's move.
	*/

	// convert the string into char *
	// const char* fenPtr = fenStr.c_str();

	// init a pointer to the fen string
	char* fen_ptr = (char*)fen;

	std::istringstream ss(fen_ptr);
	Rank rank = RANK_1;
	File file = FILE_A;

	// reset boards and state variables
	memset(bitboards, 0ULL, sizeof(bitboards));
	memset(occupancies, 0ULL, sizeof(occupancies));

	side = COLOR_NB;
	enpassant = NONE;
	castle = CASTLE_NB;

	ss >> std::noskipws;

	// 1. Piece placement
	// loop over char elements
	for (char c = *fen_ptr; c != ' '; c = *++fen_ptr)
	{
		if (c == '/') // new line
		{
			// resent file
			// go to the next rank
			file = FILE_A;
			++rank;
		}
		else
		{
			if (isdigit(c))
			{
				// leve this many empty spaces
				// c derefrences the pointer and subtracs '0' to convert it into int
				file += c - '0';
			}
			else // its a piece
			{
				Square sq = get_square(rank, file);
				Piece piece = get_piece(c);

				set_bit(bitboards[piece], sq);
				++file;
			}
		}
	}

	// 2. Side to move
	// inc pointer to castling rights and check the side to move
	(*++fen_ptr == 'w') ? (side = WHITE) : (side = BLACK);

	// go to castling rights
	fen_ptr += 2;

	// 3. Castling rights
	while (*fen_ptr != ' ')
	{
		switch (*fen_ptr++)
		{
		case 'K': castle |= WK; break;
		case 'Q': castle |= WQ; break;
		case 'k': castle |= BK; break;
		case 'q': castle |= BQ; break;
		case '-': break;
		}
	}

	// 3. Enpassant square
	if (*++fen_ptr != '-')
	{
		// parse file and rank
		int file = fen_ptr[0] - 'a';
		int rank = 8 - (fen_ptr[1] - '0');

		// init enpassant suqare
		enpassant = rank * 8 + file;
	}
	else
		enpassant = NONE;

	// populate occupancy bitboard
	for (Piece piece = WHITE_PAWN; piece <= WHITE_KING; ++piece)
	{
		// All the white pieces start from 0 to 6
		occupancies[WHITE] |= bitboards[piece];

		// All the black pieces start from 7 to 12
		occupancies[BLACK] |= bitboards[piece + 6]; // + 6 pieces
	}
}

extern inline void print_board()
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

void init_pieces()
{
	// initialize piece types from symbols
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
}

inline Piece get_piece(const char& symbol) { return pieceTypeFromSymbolMp[symbol]; }
