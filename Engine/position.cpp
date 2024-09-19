#include "position.h"

#include <iostream>
#include <sstream>
#include <memory>
#include <cstring>

namespace ChessEngine
{
	char ascii_pieces[13] = "PNBRQKpnbrqk";

	U64 bitboards[12];
	U64 occupancies[3];

	// side to move
	Color side;
	Square enpassant;
	CastlingRights castle;

	void Position::init(const char* fen)
	{
		set(fen);
		//print_board();
	}

	void set(const char* fen)
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
		// allocate memroy
		char* fen_ptr = (char*)fen;

		std::istringstream ss(fen_ptr);
		Rank rank = RANK_1;
		File file = FILE_A;

		// reset boards and state variables
		memset(bitboards, 0ULL, sizeof(bitboards));
		memset(occupancies, 0ULL, sizeof(occupancies));

		side = WHITE;
		enpassant = NONE;
		castle = CASTLE_NB;

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
					// c derefrences the pointer and subtracts '0' to convert it into int
					file += c - '0';
				}
				else // its a piece
				{
					Square sq = convert_to_square(rank, file);
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
			case 'K': castle = castle | WK; break;
			case 'Q': castle = castle | WQ; break;
			case 'k': castle = castle | BK; break;
			case 'q': castle = castle | BQ; break;
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
			enpassant = convert_to_square(rank, file);
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

		// occupanices for both sides
		occupancies[BOTH] |= occupancies[WHITE] | occupancies[BLACK];
		// occupancies[BOTH] |= occupancies[BLACK];
	}

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

			std::cout << "\n";
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

	Piece get_piece(const char& symbol)
	{
		switch (symbol)
		{
		case 'p': return BLACK_PAWN;
		case 'n': return BLACK_KNIGHT;
		case 'b': return BLACK_BISHOP;
		case 'r': return BLACK_ROOK;
		case 'q': return BLACK_QUEEN;
		case 'k': return BLACK_KING;

		case 'P': return WHITE_PAWN;
		case 'N': return WHITE_KNIGHT;
		case 'B': return WHITE_BISHOP;
		case 'R': return WHITE_ROOK;
		case 'Q': return WHITE_QUEEN;
		case 'K': return WHITE_KING;
		}

		return EMPTY;
	}

	// check if square is attacked
	bool is_square_attacked(const Square& square, const Color color)
	{
		// !color -> white

		// This gets the pawn attacks at the white side and square
		// then applies bitwise AND to the opposide piece
		bool is_pawn_attacks = !color
			? pawn_attacks[BLACK][square] & bitboards[WHITE_PAWN]
			: pawn_attacks[WHITE][square] & bitboards[BLACK_PAWN];

		bool is_knight_attacks = attacks_bb_by<KNIGHT>(square)
			& bitboards[
				!color
					? WHITE_KNIGHT
					: BLACK_KNIGHT];

		bool is_king_attacks = attacks_bb_by<KING>(square)
			& bitboards[
				!color
					? WHITE_KING
					: BLACK_KING];

		bool is_bishop_attacks = attacks_bb_by<BISHOP>(square, occupancies[BOTH])
			& bitboards[
				!color
					? WHITE_BISHOP
					: BLACK_BISHOP];

		bool is_rook_attacks = attacks_bb_by<ROOK>(square, occupancies[BOTH])
			& bitboards[
				!color
					? WHITE_ROOK
					: BLACK_ROOK];

		bool is_queen_attacks = attacks_bb_by<QUEEN>(square, occupancies[BOTH])
			& bitboards[
				!color
					? WHITE_QUEEN
					: BLACK_QUEEN];

		return is_pawn_attacks ||
			is_knight_attacks ||
			is_king_attacks ||
			is_bishop_attacks ||
			is_rook_attacks ||
			is_queen_attacks;
	}

	void print_attacked_squares(Color color)
	{
		std::cout << std::endl;

		for (Rank rank = RANK_1; rank <= RANK_8; ++rank)
		{
			for (File file = FILE_A; file <= FILE_H; ++file)
			{
				Square square = convert_to_square(rank, file);

				if (!file)
					printf(" %d ", 8 - rank);

				printf(" %d", is_square_attacked(square, color) ? 1 : 0);
			}

			std::cout << std::endl;
		}

		printf("\n    a b c d e f g h\n\n");
	}
}