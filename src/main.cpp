#include <iostream>
#include <numeric>

#include "position.h"
#include "bitboard.h"
#include "perft.h"
#include "uci.h"
#include "score.h"

using namespace KhaosChess;

int main()
{
	Bitboards::init();

	bool debug = true;

	if (debug)
	{
		std::cout << "Debugging\n"
				  << std::endl;

		Position pos{};
		InfoListPtr infos(new std::deque<MoveInfo>(1));

		// pin position: "3r4/bk6/8/r1pPK3/8/8/6B1/8 w - - 0 1";
		// pos.set(TEST_FEN, &infos->back());

		// std::cout << pos;

		std::string white_queen_up = "r1bqkbnr/pppp1ppp/2n5/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 0 1";
		std::string white_with_two_rooks_central_advantage = "r1bq1rk1/pppp1ppp/2n2n2/1B2p3/4P3/2N2N2/PPPP1PPP/R1BQ1RK1 w - - 0 1";
		std::string eq_pos = "rnbqkbnr/pp1ppppp/2p5/8/8/2P5/PP1PPPPP/RNBQKBNR w KQkq - 0 2";
		std::string queen_down_for_w = "r3kq2/pb2ppb1/2pp4/8/2P5/1B6/PP3PPP/3R1K1R w q - 0 1";
		std::string slightly_better_for_black = "r3k2r/pb1nppb1/2pp1n2/8/2P5/5Q2/PP3PPP/3R1K1R w q - 0 1";

		// pos.set(white_queen_up, &infos->back());
		// std::cout << Scorer<SC_ALL>().get_score(pos) << "\n";
		// Scorer<SC_ALL>().print_stats(pos);

		// pos.set(white_with_two_rooks_central_advantage, &infos->back());
		// std::cout << "Score: " << Scorer<SC_ALL>().get_score(pos) << "\n";
		// Scorer<SC_ALL>().print_stats(pos);

		// pos.set(eq_pos, &infos->back());
		// std::cout << "Score: " << Scorer<SC_ALL>().get_score(pos) << "\n";
		// Scorer<SC_ALL>().print_stats(pos);

		// pos.set(queen_down_for_w, &infos->back());
		// std::cout << "Score: " << Scorer<SC_ALL>().get_score(pos) << "\n";
		// Scorer<SC_ALL>().print_stats(pos);

		pos.set(slightly_better_for_black, &infos->back());
		// Scorer<SC_ALL>(pos).print_stats();
		// std::cout << "Score: " << Scorer<SC_ALL>().get_score(pos) << "\n";
	}
	else
		uci_loop();

	// system("pause");
	return 0;
}