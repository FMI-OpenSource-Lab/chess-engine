#ifndef UCI_H
#define UCI_H

#include <string>

namespace ChessEngine
{
	// parse user/GUI move string input (e.g. "e7e8q")
	extern int parse_move(const char* move_string);
	extern void parse_position(const char *cmd);
	extern void parse_go(const char *cmd);
	extern void uci_loop();
}
#endif // !UCI_H
