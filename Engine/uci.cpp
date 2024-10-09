#include "uci.h"
#include "movegen.h"
#include "move.h"
#include "position.h"
#include "search.h"

#include <iostream>
#include <stdlib.h>

namespace ChessEngine
{
	/*
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
							&& move_string[4] == tolower(ascii_pieces[p]))
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

		print_board();
	}

	void parse_go(const char* cmd)
	{
		int depth = -1;
		const char* current = cmd;

		// fixed depth search
		if (current = strstr(cmd, "depth"))
			depth = atoi(current + 6);
		else
			depth = 6;

		// std::cout << "depth " << depth << "\n";

		search_position(depth);
	}
	*/

	/*
		GUI -> isready
		Engine -> readyok
		GUI -> ucinewgame
	*/
	/*
		void uci_loop()
		{
			constexpr auto INPUT_BUFFER = 10000;;

			// rest stdin & stdout buffers
			std::setvbuf(stdin, NULL, _IONBF, 0);
			std::setvbuf(stdout, NULL, _IONBF, 0);

			// def user/GUI inout buffer
			char input_buffer[INPUT_BUFFER];

			std::cout << "id name ChessEngine\n";
			std::cout << "id author Iulian Dashev\n";
			std::cout << "uciok\n";

			// main loop
			while (true)
			{
				// rest user/GUI input
				memset(input_buffer, 0, sizeof(input_buffer));

				// making sure output reacehes GUI
				fflush(stdout);

				// get user/GUI input
				if (!fgets(input_buffer, INPUT_BUFFER, stdin))
					continue;

				// available input
				if (input_buffer[0] == '\n')
					continue;

				// parse UCI "isready" command
				if (strncmp(input_buffer, "isready", 7) == 0)
				{
					std::cout << "readyok\n";
					continue;
				}

				// parse UCI "position" command
				else if (strncmp(input_buffer, "position", 8) == 0)
					parse_position(input_buffer);

				// parse UCI "ucinewgame" command
				else if (strncmp(input_buffer, "ucinewgame", 10) == 0)
					parse_position("position startpos");

				// parse UCI "go" command
				else if (strncmp(input_buffer, "go", 2) == 0)
					parse_go(input_buffer);

				// parse UCI "quit" command
				else if (strncmp(input_buffer, "quit", 4) == 0)
					break; // exit loop

				// parse UCI "uci" command
				else if (strncmp(input_buffer, "uci", 3) == 0)
				{
					std::cout << "\nid name ChessEngine\n";
					std::cout << "id author Iulian Dashev\n";
					std::cout << "uciok\n";
				}

				else if (!strncmp(input_buffer, "d", 1)) print_board();
			}
		}
		*/
}