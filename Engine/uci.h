#ifndef UCI_H
#define UCI_H

#include <string>

namespace ChessEngine
{
	// parse user/GUI move string input (e.g. "e7e8q")
	extern int parse_move(const char* move_string);
	extern void parse_position(const char *cmd);



}
#endif // !UCI_H
