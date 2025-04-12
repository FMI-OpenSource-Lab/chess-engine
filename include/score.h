#pragma once

#include "movegen.h"
#include "defs.h"
#include "consts.h"
#include "position.h"

#include <array>
#include <iomanip>

namespace KhaosChess
{
	// Evaluation weights for different components
	struct EvalWeights
	{
		Value material = 100;	   // Base weight for material
		Value position = 80;	   // PST scores
		Value mobility = 60;	   // Piece mobility
		Value king_safety = 100;   // King safety factor
		Value pawn_structure = 50; // Pawn structure importance
		Value connectivity = 40;   // Piece coordination/connectivity
	};

	struct Score
	{
		Value mg; // Middlegame score
		Value eg; // Endgame score

		constexpr Score() : mg(0), eg(0) {}
		constexpr Score(Value mg, Value eg) : mg(mg), eg(eg) {}
		constexpr Score(Value v) : mg(v), eg(v) {}
		Score(const Score &other) = default;
		Score(Score &&other) = default;

		constexpr Score &operator=(const Score &other) = default;
		constexpr Score &operator=(Score &&other) = default;

		// Addition operators
		constexpr Score operator+(const Score &other) const { return Score(mg + other.mg, eg + other.eg); }
		constexpr Score &operator+=(const Score &other)
		{
			mg += other.mg;
			eg += other.eg;
			return *this;
		}

		// Subtraction operators
		constexpr Score operator-(const Score &other) const { return Score(mg - other.mg, eg - other.eg); }
		constexpr Score &operator-=(const Score &other)
		{
			mg -= other.mg;
			eg -= other.eg;
			return *this;
		}

		// Multiplication operators
		constexpr Score operator*(const Score &other) const { return Score(mg * other.mg, eg * other.eg); }

		// Division operators
		constexpr Score operator/(const Score &other) const { return Score(mg / other.mg, eg / other.eg); }

		// Get interpolated score based on game phase
		constexpr Value interpolate(int phase) const { return ((mg * (MAX_PHASE_SCORE - phase)) + (eg * phase)) / MAX_PHASE_SCORE; }
	};

	constexpr Score operator*(const Value &value, const Score &other) { return Score(other.mg * value, other.eg * value); }

	inline std::ostream &operator<<(std::ostream &os, Score s)
	{
		return os << std::setw(6) << std::setfill(' ') << s.mg << " "
				  << std::setw(6) << std::setfill(' ') << s.eg << " ";
	}

	constexpr Score S(Value mg, Value eg) { return Score(mg, eg); }

	// Material values
	struct Material
	{
		Score piece_value[PIECE_TYPE_NB] = {
			S(0, 0), S(300, 370), S(890, 880), S(900, 950), S(1400, 1550), S(2900, 2800), S(0, 0)};

		constexpr Value all_pieces() const
		{
			return 2 * (8 * piece_value[PAWN] +	  // 8 PAWNS
						2 * piece_value[KNIGHT] + // 2 KNIGHTS
						2 * piece_value[BISHOP] + // 2 BISHOPS
						2 * piece_value[ROOK] +	  // 2 ROOKS
						1 * piece_value[QUEEN])
						   .eg; // 1 QUEEN
		}
	};

	// Piece-specific bonuses and penalties
	struct PieceBonus
	{
		// Bonuses
		Score mobility[PIECE_TYPE_NB] = {
			S(0, 0), S(5, 10), S(12, 24), S(18, 8), S(6, 24), S(4, 12), S(0, 10)};

		Score bishop_pair = S(50, 60);
		Score connected_rooks = S(20, 10);
		Score outpost_knight = S(25, 10);
		Score outpost_bishop = S(20, 10);
		Score safe_knight = S(10, 2);
		Score control_center_knight = S(10, 10);
		Score vulnerable_queen = S(-30, -15);

		Score rook_open_file = S(20, 40);
		Score rook_semi_open_file = S(10, 11);

		Score control_space[PIECE_TYPE_NB] = {
			S(0, 0), S(20, 30), S(20, 0), S(10, 5), S(10, 10), S(10, 20), S(0, 0)};

		// Penalties
		Score trapped_rook = S(-50, -10);
		
		Score pinned_penalty[PIECE_TYPE_NB] = {
			S(0, 0), S(-20, -20), S(-10, -15), S(-10, -20), S(0, 0), S(0, 0), S(0, 0)};
	};

	// Pawn structure bonuses and penalties
	struct PawnStructure
	{
		// Bonuses
		Value passed_rank_weight[RANK_NB] = {0, 1, 1, 2, 3, 6, 10, 0};
		Value connected_bonus[RANK_NB] = {0, 0, 2, 5, 20, 40, 80, 0};

		Score control_center = S(30, 30);
		Score passed = S(20, 40);

		// Penalties
		Score doubled = S(-15, -45);
		Score backward = S(-30, -100);
		Score isolated = S(-20, -80);
		Score pawn_island = S(-10, -20);
		Score same_color_as_bishop = S(-3, -5);
	};

	// King safety related bonuses and penalties
	struct KingSafety
	{
		Score bonus = S(30, 0);

		// Penalties
		Score weak_diagonals = S(-5, 0);
		Score weak_lines = S(-7, 0);
		Score weak_backrank = S(-75, -100);
		Score pawn_proximity_penalty = S(0, -5);

		Score protector_penalty[PIECE_TYPE_NB] = {
			S(0, 0), S(0, 0), S(-6, -4), S(-5, -3), S(0, 0), S(0, 0), S(0, 0)};

		Score attacker_penalty[PIECE_TYPE_NB] = {
			S(0, 0), S(0, 0), S(-7, -4), S(-4, -3), S(0, 0), S(0, 0), S(0, 0)};
	};

	// Define the actual instances
	constexpr Material MATERIAL_SCORES;
	constexpr PieceBonus PIECE_SCORES;
	constexpr PawnStructure PAWN_STRUCTURE_SCORES;
	constexpr KingSafety KING_SAFETY_SCORES;

	constexpr Value VALUE_ALL_PIECES = MATERIAL_SCORES.all_pieces();

	constexpr Score MOBILITY_BONUS[PIECE_TYPE_NB] = {
		PIECE_SCORES.mobility[0], PIECE_SCORES.mobility[1], PIECE_SCORES.mobility[2],
		PIECE_SCORES.mobility[3], PIECE_SCORES.mobility[4], PIECE_SCORES.mobility[5],
		PIECE_SCORES.mobility[6]};

	class Position;

	enum ScoreComponent
	{
		SC_MATERIAL,		   // The material scores
		SC_MOBILITY,		   // The mobility scores
		SC_KING_SAFETY,		   // The king safety scores
		SC_PAWN_STRUCTURE,	   // The pawn structure scores
		SC_PIECE_COORDINATION, // The piece coordination scores
		SC_ALL				   // Combined score
	};

	template <ScoreComponent>
	Score total_scores(const Position &pos, Score &score);

	template <ScoreComponent T>
	struct Scorer
	{
		explicit Scorer(const Position &pos) : total(total_scores<T>(pos, score)) {}
		void print_stats();

	private:
		Score total, score;
	};
} // namespace KhaosChess
