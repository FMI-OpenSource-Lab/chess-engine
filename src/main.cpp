#include <iostream>

#include "position.h"
#include "bitboard.h"
#include "perft.h"
#include "uci.h"
#include "score.h"

using namespace KhaosChess;

int main()
{
	Bitboards::init();
	Eval::init();

	bool debug = true;

	if (debug)
	{
		std::cout << "Debugging\n"
				  << std::endl;

		Position pos{};
		InfoListPtr infos(new std::deque<MoveInfo>(1));

		// pin position: "3r4/bk6/8/r1pPK3/8/8/6B1/8 w - - 0 1";
		pos.set(TEST_FEN, &infos->back());

		std::cout << pos;

		std::string unsafe_king = "r1bqkb1r/pppp1ppp/2n2n2/4p3/2B1P3/5Q2/PPPP1PPP/RNB1K1NR w KQkq - 0 1";
		std::string safe_king = "r1bqkb1r/pppp1ppp/2n2n2/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 0 1";

		std::cout << "King safety test:\n";

		Value unsafe_safety = Eval::king_safety<WHITE>(pos);
		std::cout << "Unsafe king: " << unsafe_safety << "\n";

		Value safe_safety = Eval::king_safety<WHITE>(pos);
		std::cout << "Safe king: " << safe_safety << "\n";
		std::cout << "Safe king should be higher than unsafe king\n\n";

		std::cout << "Full evaluation test:\n";

		std::string white_advantage = "r1bqkb1r/ppp2ppp/2np1n2/4p3/2BPP3/5N2/PPP2PPP/RNBQK2R w KQkq - 0 1";
		std::cout << "White advantage: " << Eval::evaluate(pos) << " (should be positive)\n";

		std::string black_advantage = "rnbqkb1r/pp3ppp/2p2n2/3pp3/3PP3/P1P5/2P2PPP/RNBQKBNR w KQkq - 0 1";
		std::cout << "Black advantage: " << Eval::evaluate(pos) << " (should be negative)\n";

		std::string eq_pos = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
		std::cout << "Equal position: " << Eval::evaluate(pos) << " (should be near zero)\n\n";

		std::string passed_pawn = "8/3P4/8/8/8/8/8/k6K w - - 0 1";
		std::cout << "Passed pawn test: " << Eval::pawn_structure<WHITE>(pos) << "\n";

		std::string bishop_pair = "8/8/8/8/8/8/8/k1BB3K w - - 0 1";
		std::cout << "Bishop pair: " << Eval::piece_coordination<WHITE>(pos) << "\n";

		std::string single_bishop = "8/8/8/8/8/8/8/k1B4K w - - 0 1";
		std::cout << "Single bishop: " << Eval::piece_coordination<WHITE>(pos) << "\n";
	}
	else
		uci_loop();

	system("pause");
	return 0;
}