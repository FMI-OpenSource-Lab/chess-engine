#define NOMINMAX

#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <vector>
#include <unordered_map>
#include <iomanip>
#include <iterator>
#include <set>
#include <sstream>

constexpr auto RED = "\033[1;31m"; // Red text;
constexpr auto RESET = "\033[0m"; // Reset color;
constexpr auto WHITE_BG = "\033[47m"; // White background;

constexpr auto UNDERLINE = "\033[4m"; // Underline text;

using Pair = std::pair<std::string, uint64_t>;

void compare_results()
{
	std::string results_str{};

	std::ifstream ce_results("../results.txt");
	std::ifstream sf_results("../sf_results.txt");

	std::vector<Pair> sf{};
	std::vector<Pair> ce{};

	while (std::getline(ce_results, results_str))
	{
		std::string key = results_str.substr(0, 4);
		uint64_t value = std::stoull(results_str.substr(6, results_str.size()));

		auto pair = Pair(key, value);

		ce.emplace_back(pair);
	}

	ce_results.close();

	while (std::getline(sf_results, results_str))
	{
		std::string key = results_str.substr(0, 4);
		uint64_t value = std::stoull(results_str.substr(6, results_str.size()));

		auto pair = Pair(key, value);

		sf.emplace_back(pair);
	}

	sf_results.close();

	std::sort(ce.begin(), ce.end());
	std::sort(sf.begin(), sf.end());

	std::vector<Pair>::iterator it1 = ce.begin();
	std::vector<Pair>::iterator it2 = sf.begin();

	const int key_width = 10;
	const int value_width = 8;

	std::cout << std::left << std::setw(key_width) << "CE Moves"
		<< " | " << std::setw(key_width) << "SF Moves"
		<< " | " << std::setw(value_width) << "CE Nodes"
		<< " | " << std::setw(value_width) << "SF Nodes"
		<< std::endl;

	std::cout << std::string(45, '-') << std::endl;

	while (it1 != ce.end() || it2 != sf.end())
	{
		bool isDifferent = false;
		std::string key1 = "----", key2 = "----";
		uint64_t val1 = 0, val2 = 0;

		if (it1 != ce.end() && (it2 == sf.end() || it1->first < it2->first))
		{
			key1 = it1->first;
			val1 = it1->second;
			isDifferent = true; // Different because key is missing in vec2
			++it1;
		}
		else if (it2 != sf.end() && (it1 == ce.end() || it1->first > it2->first))
		{
			key2 = it2->first;
			val2 = it2->second;
			isDifferent = true; // Different because key is missing in vec1
			++it2;
		}
		else
		{
			key1 = it1->first;
			key2 = it2->first;
			val1 = it1->second;
			val2 = it2->second;

			if (val1 != val2) isDifferent = true; // Different because values mismatch

			++it1;
			++it2;
		}

		if (isDifferent) std::cout << UNDERLINE << RED;

		// Print row with proper alignment
		std::cout << std::left << std::setw(key_width) << key1
			<< " | " << std::setw(key_width) << key2
			<< " | " << std::setw(value_width) << val1
			<< " | " << std::setw(value_width) << val2
			<< " | " << RESET << std::endl;
	}
}

int main()
{
	int depth = 5;

	do
	{
		compare_results();
		std::cin.get();
	} while (depth-- != 0);

	return 0;
}