#define NOMINMAX

#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <vector>
#include <iomanip>

#include "../Engine/defs.h"

int main()
{
	std::string results_str{};

	std::ifstream ce_results("../results.txt");
	std::ifstream sf_results("../sf_results.txt");

	std::vector<std::string> sf{};
	std::vector<std::string> ce{};

	ce.reserve(MAX_MOVES);
	while (std::getline(ce_results, results_str))
		ce.emplace_back(results_str);

	ce_results.close();

	sf.reserve(MAX_MOVES);
	while (std::getline(sf_results, results_str))
		sf.emplace_back(results_str);

	sf_results.close();

	std::sort(ce.begin(), ce.end());
	std::sort(sf.begin(), sf.end());

	const int ce_size = ce.size();
	const int sf_size = sf.size();

	const int total = std::max(ce_size, sf_size);

	std::cout << "Moves" << " | " << " sf     " << " | " << " ce" << "\n\n";

	if (!std::abs(ce_size - sf_size)) // equal sizes
	{
		for (int i = 0; i < ce_size; i++)
		{
			std::string ce_move = ce[i].substr(0, 4);
			std::string sf_move = sf[i].substr(0, 4);

			if (ce_move == sf_move) // matching moves
			{
				std::string ce_nodes = ce[i].substr(6, ce[i].size());
				std::string sf_nodes = sf[i].substr(6, sf[i].size());

				if (ce_nodes != sf_nodes) // Print differences
					std::cout << ce_move << "  |  " << sf_nodes << " |  " << ce_nodes << "\n";
			}
			else // not matching moves
			{
				// e1c1: 3551583

				// check from next element, since current moves aren't equal
				for (int j = i; j < ce_size; j++)
				{
					std::string sf_next_move = sf[j].substr(0, 4);

					if (ce_move == sf_next_move) // if a match is found on the next moves
					{
						// std::cout << ce_move << "  |  " << sf_next_move << "\n";
						
						std::swap(ce[i], sf[j]);
						break;
					}
				}

				std::cout << sf[i].substr(0, 4) << "  |  " << ce[i].substr(0, 4) << "\n";
			}
		}
	}
	else // diff sizes
	{

	}


	return 0;
}