#include "bitboard.h"
#include "attacks.h"
#include "position.h"

#include <iostream>

inline bool is_attacked(const Square& square, const Color& side_to_move);
extern void print_attacked_squares(Color& colour);

U64 p_attacks[2][64];
U64 n_attacks[64];
U64 k_attacks[64];

void init_all()
{
	// Initialize attacks
	Attacks::init();

	// Initialize bitboards
	Bitboards::init();

	// load the starting fen
	Position::init(TEST_ATTACKS_FEN);
}

// check if square is attacked
inline bool is_attacked(const Square& square, const Color& side_to_move)
{
	// variables that check if a square is attacked by a piece
	// checking the pawn attack table at square s and apply attack mask on the board that corresponds with the either side of pawns

	// side_to_move ? get_piece(UPPERCASE SYMBOL) : get_piece(lowercase symbol) means that if white are attacking White pawn attack mask will be used, and similiar for black

	bool isWhite = side_to_move == WHITE;

	Piece get_pawn_piece = isWhite
		? get_piece('P') // if is White playing
		: get_piece('p'); // if Black is playing

	bool is_pawn_square_attacked =
		pawnAttacks[isWhite ? BLACK : WHITE][square] &
		bitboards[get_pawn_piece];

	bool is_knight_square_attacked =
		knightAttacks[square] &
		bitboards[
			(int)side_to_move
				? get_piece('N') // for White
				: get_piece('n')]; // for Black

	if (is_pawn_square_attacked || is_knight_square_attacked)
		return true;

	return false;
}

extern void print_attacked_squares(Color& colour)
{
	std::cout << std::endl;

	for (Rank rank = RANK_1; rank <= RANK_8; ++rank)
	{
		for (File file = FILE_A; file <= FILE_H; ++file)
		{
			Square square = get_square(rank, file);

			if (!file)
				printf(" %d ", 8 - rank);

			printf(" %d", is_attacked(square, colour) ? 1 : 0);
		}

		std::cout << std::endl;
	}

	printf("\n    a b c d e f g h\n\n");
}

int main()
{
	init_all();

	Color c = WHITE;
	print_attacked_squares(c);

	system("pause");
	return 0;
}