#include "uci.h"
#include "movegen.h"
#include "move.h"
#include "position.h"

#include <iostream>

namespace ChessEngine
{
	int parse_move(const char* move_string)
	{
		moves move_list[1];
		generate_moves(move_list);

		// parse source
		int source = (move_string[0] - 'a') + (8 - (move_string[1] - '0')) * 8;
		int target = (move_string[2] - 'a') + (8 - (move_string[3] - '0')) * 8;

		for (int move_c = 0; move_c < move_list->count; move_c++)
		{
			int move = move_list->moves[move_c];

			// source & target are available
			if (source == get_move_source(move)
				&& target == get_move_target(move))
			{
				int promoted = get_move_promoted(move);

				// if there is promoted move
				if (promoted)
				{
					// iterate through promoted piece
					for (Piece p = WHITE_QUEEN; p >= WHITE_KNIGHT; --p)
						// promoted to Queen, Rook, Bishop, Knight
						if ((promoted == p || promoted == (p + 6))
							&& move_string[4] == ascii_pieces[p])
							return move;

					continue;
				}

				// legal move
				return move;
			}
		}

		// illigal move
		return 0;
	}

	void parse_position(const char* cmd)
	{
		// shift to next token
		cmd += 9;
		const char* current = cmd;
		const char* fen;

		if (strncmp(current, "startpos", 8) == 0)
			Position::init(START_FEN);
		else // UCI "fen" command
		{
			// fen command is available
			current = strstr(cmd, "fen");

			// fen command is not available
			if (current == NULL)
				Position::init(START_FEN);
			else // found FEN
			{
				current += 4;

				// init board position from FEN command
				Position::init(current);
				fen = current;
			}
		}

		// parse moves command
		current = strstr(cmd, "moves");

		// moves available
		if (current != NULL)
		{
			// shift to the moves
			current += 6;

			while (*current)
			{
				// parse next move
				int move = parse_move(current);

				// no move
				if (move == 0)
					break;

				// make the move
				make_move(move, MT_NORMAL);

				while (*current && *current != ' ') current++;

				current++;
			}
		}
	}
}