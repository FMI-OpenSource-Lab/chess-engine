#include "uci.h"
#include "movegen.h"
#include "move.h"
#include "position.h"
#include "search.h"
#include "perft.h"

#include <iostream>
#include <stdlib.h>

namespace ChessEngine
{
	Move parse_move(std::string move_string, const Position& pos)
	{
		Square source = Square((move_string[0] - 'a') + (8 - (move_string[1] - '0')) * 8);
		Square target = Square((move_string[2] - 'a') + (8 - (move_string[3] - '0')) * 8);

		for (auto& m : MoveList<GT_LEGAL>(pos))
		{
			if (m.source_square() == source
				&& m.target_square() == target)
			{
				if (m.move_type() == MT_PROMOTION)
				{
					PieceType p = m.promoted();

					for (PieceType pt : {KNIGHT, BISHOP, ROOK, QUEEN})
						if (p == pt && move_string[4] == ascii_pieces[get_piece(pos.side_to_move(), pt)])
							return m;

					continue;
				}

				return m;
			}
		}

		return Move::invalid_move();
	}

	void parse_position(const char* cmd, Position& pos, MoveInfo& mi)
	{
		// shift to next token, becase "position" is 8 characters and " " is 1, hence shift 9
		cmd += 9;
		const char* current = cmd;

		if (strncmp(current, "startpos", 8) == 0)
			pos.set(START_FEN, &mi);

		else // UCI "fen" command
		{
			// fen command is available
			current = strstr(cmd, "fen");

			// fen command is not available
			if (current == NULL)
				pos.set(START_FEN, &mi);
			else // found FEN
			{
				current += 4;

				// init board position from FEN command
				pos.set(current, &mi);
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
				Move move = parse_move(current, pos);

				// no move
				if (move == Move::invalid_move())
					break;

				// make the move
				pos.do_move(move, mi);

				while (*current && *current != ' ') current++;

				current++;
			}
		}
	}

	void parse_go(const char* cmd, Position& pos)
	{
		int depth = 5;
		const char* current = cmd;

		current += 3;

		// fixed depth search
		if (current = strstr(cmd, "depth"))
		{
			depth = atoi(current + 6);
			// search_position(depth);
		}
		else if (current = strstr(cmd, "perft"))
		{
			depth = atoi(current + 6);
			std::cout << "\nNodes: " << perft<true>(pos, depth) << std::endl;
		}
		else
			std::cout << "Invalid command after go!\nTry go perft <number>\n" << current;

		// std::cout << "depth " << depth << "\n";
	}

	/*
		GUI -> isready
		Engine -> readyok
		GUI -> ucinewgame
	*/
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

		InfoListPtr infos(new std::deque<MoveInfo>(1));
		Position pos;

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
				parse_position(input_buffer, pos, infos->back());

			// parse UCI "ucinewgame" command
			else if (strncmp(input_buffer, "ucinewgame", 10) == 0)
				parse_position("position startpos", pos, infos->back());

			// parse UCI "go" command
			else if (strncmp(input_buffer, "go", 2) == 0)
				parse_go(input_buffer, pos);

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

			else if (!strncmp(input_buffer, "d", 1))
				std::cout << pos << std::endl;
		}
	}
}