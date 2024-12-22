#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <vector>>
#include <iomanip>

#include "../Engine/defs.h"

int main()
{
	std::string results_str{};

	std::ifstream ml_results("../results.txt");
	std::ifstream sf_results("../sf_results.txt");

	std::vector<std::string> sf{};
	std::vector<std::string> ml{};

	ml.reserve(MAX_MOVES);
	while (std::getline(ml_results, results_str))
		ml.emplace_back(results_str);

	ml_results.close();

	sf.reserve(MAX_MOVES);
	while (std::getline(sf_results, results_str))
		sf.emplace_back(results_str);

	sf_results.close();

	std::sort(ml.begin(), ml.end());
	std::sort(sf.begin(), sf.end());

	const auto& bigger = ml.size() > sf.size() ? ml : sf;
	const auto& smaller = ml.size() > sf.size() ? sf : ml;

	std::cout << (bigger == ml ? "ce" : "sf")
		<< "         " << (smaller == sf ? "sf" : "ml")
		<< "\n" << std::string(25, '-') << std::endl;

	for (int i = 0; i < bigger.size(); i++)
	{
		if (i < smaller.size())
		{
			if (bigger[i] != smaller[i])
				std::cout << bigger[i] << " | " << smaller[i] << std::endl;
		}
		else
			std::cout << bigger[i] << " | " << "----" << std::endl;
	}

	return 0;
}