#pragma once

#include <string>

#include "position.h"

namespace KhaosChess
{
	class Position;
	class Move;

	// parse user/GUI move string input (e.g. "e7e8q")
	extern Move parse_move(std::string move_string, const Position& pos);
	extern void parse_position(const char* cmd, Position& pos, MoveInfo& mi);
	extern void parse_go(const char* cmd, Position& pos);
	extern void uci_loop();
}
