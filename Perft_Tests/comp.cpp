#define NOMINMAX

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstdio>
#include <algorithm>
#include <iomanip>
#include <cstdint>
#include <thread>
#include <chrono>

constexpr auto RED = "\033[1;31m"; // Red text;
constexpr auto RESET = "\033[0m"; // Reset color;
constexpr auto WHITE_BG = "\033[47m"; // White background;

constexpr auto UNDERLINE = "\033[4m"; // Underline text;

using Pair = std::pair<std::string, uint64_t>;

void read_stockfish(std::string& fen, int depth, std::string& out_filename, std::string moves = "")
{
	std::ofstream out_file(out_filename);

	if (!out_file)
	{
		std::cerr << "Failed to open file: " << out_filename << std::endl;
		return;
	}

	FILE* fp = _popen("C:\\stockfish\\stockfish.exe", "w");

	if (!fp)
	{
		std::cerr << "Failed to run stockfish!" << std::endl;
		return;
	}

	if (!moves.empty())
		fprintf(fp, "position fen %s moves %s\n", fen.c_str(), moves.c_str());
	else
		fprintf(fp, "position fen %s\n", fen.c_str());

	fprintf(fp, "go perft %d\n", depth);
	fflush(fp);

	std::string output;
	char buffer[128];
	int count = 0;

	while (fgets(buffer, sizeof(buffer), fp) != nullptr)
	{
		count++;

		if (count > 4)
			out_file << buffer;  // Write to file only from the 5th line onward
	}

	_pclose(fp);
	out_file.close();
}

void print_differences(std::string& ce_filename, std::string& sf_filename)
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

	while (it1 != ce.end() || it2 != sf.end()) {
		bool isDifferent = false;
		std::string key1 = "----", key2 = "----";
		uint64_t val1 = 0, val2 = 0;

		if (it1 != ce.end() && (it2 == sf.end() || it1->first < it2->first)) {
			key1 = it1->first;
			val1 = it1->second;
			isDifferent = true; // Different because key is missing in vec2
			++it1;
		}
		else if (it2 != sf.end() && (it1 == ce.end() || it1->first > it2->first)) {
			key2 = it2->first;
			val2 = it2->second;
			isDifferent = true; // Different because key is missing in vec1
			++it2;
		}
		else {
			key1 = it1->first;
			key2 = it2->first;
			val1 = it1->second;
			val2 = it2->second;

			if (val1 != val2) isDifferent = true; // Different because values mismatch

			++it1;
			++it2;
		}

		if (isDifferent)
			std::cout << UNDERLINE << RED;

		// Print row with proper alignment
		std::cout << std::left << std::setw(key_width) << key1
			<< " | " << std::setw(key_width) << key2
			<< " | " << std::setw(value_width) << val1
			<< " | " << std::setw(value_width) << val2
			<< " | " << RESET << std::endl;
	}
}

const int START_DEPTH_VALUE = 5;
int main()
{
	std::string fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 ";
	int depth = START_DEPTH_VALUE;

	std::string ce_filename = "../results.txt";
	std::string sf_filename = "../sf_results.txt";

	std::string moves = "";

	read_stockfish(fen, depth--, sf_filename, moves);

	print_differences(ce_filename, sf_filename);

	while (depth != 0)
	{
		if (depth < START_DEPTH_VALUE)
		{
			std::string curr;

			std::cout << "Enter move: ";
			std::cin >> curr;

			moves = moves + " " + curr;
		}

		read_stockfish(fen, depth, sf_filename, moves);

		print_differences(ce_filename, sf_filename);

		depth--;

		std::cin.get();
	}

	return 0;
}