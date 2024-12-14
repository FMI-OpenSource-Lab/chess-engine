#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <vector>
#include <map>
#include <unordered_map>
#include <iomanip>

int main()
{
	std::string results_str{};

	std::ifstream results("../results.txt");
	std::ifstream sf_results("../sf_results.txt");

	std::map <std::string, int> sf{};
	std::map<std::string, int> ml{};

	std::string delimiter = ": ";
	while (std::getline(results, results_str))
	{
		std::string key = results_str.substr(0, results_str.find(delimiter));
		std::string value = results_str.substr(results_str.find(delimiter) + 2, results_str.length());

		ml.insert({ key, std::stoi(value) });
	}

	results.close();

	while (std::getline(sf_results, results_str))
	{
		std::string key = results_str.substr(0, results_str.find(delimiter));
		int value = std::stoi(results_str.substr(results_str.find(delimiter) + 2, results_str.length()));

		sf.insert({ key, value });
	}

	sf_results.close();

	std::cout << "Move  Correct  Wrong" << '\n';
	std::cout << "---------------------\n";

	// Process map1 (Your moves)
	for (const auto& pair : ml) {
		auto it = sf.find(pair.first);
		if (it == sf.end()) {
			// Move exists only in map1 (no correct Stockfish equivalent)
			std::cout << std::left << std::setw(8) << pair.first
				<< std::setw(8) << pair.second
				<< std::setw(8) << "-" << '\n';
		}
		else if (pair.second != it->second) {
			// Move exists in both maps but values differ
			std::cout << std::left << std::setw(8) << pair.first
				<< std::setw(8) << pair.second
				<< std::setw(8) << it->second << '\n';
		}
	}

	// Process map2 for moves only in Stockfish's map
	for (const auto& pair : sf) {
		auto it = ml.find(pair.first);
		if (it == ml.end()) {
			// Move exists only in map2 (correct move not present in your moves)
			std::cout << std::left << std::setw(8) << pair.first
				<< std::setw(8) << "-"
				<< std::setw(8) << pair.second << '\n';
		}
	}

	return 0;
}