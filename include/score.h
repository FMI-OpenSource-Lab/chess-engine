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
			return 2 * (8 * S(300, 370).eg +   // 8 PAWNS
						2 * S(890, 880).eg +   // 2 KNIGHTS
						2 * S(900, 950).eg +   // 2 BISHOPS
						2 * S(1400, 1550).eg + // 2 ROOKS
						1 * S(2900, 2800).eg); // 1 QUEEN
		}
	};

	// Piece-specific bonuses and penalties
	struct PieceBonus
	{
		Score mobility[PIECE_TYPE_NB] = {
			S(0, 0), S(5, 10), S(12, 24), S(18, 8), S(6, 24), S(4, 12), S(0, 10)};

		Score pinned_penalty[PIECE_TYPE_NB] = {
			S(0, 0), S(-20, -20), S(-10, -15), S(-10, -20), S(0, 0), S(0, 0), S(0, 0)};

		Score bishop_pair = S(50, 60);
		Score connected_rooks = S(20, 10);
		Score outpost_knight = S(25, 10);
		Score outpost_bishop = S(20, 10);
		Score safe_knight = S(10, 2);
		Score control_center_knight = S(10, 10);
		Score vulnerable_queen = S(-30, -15);

		Score rook_open_file = S(20, 40);
		Score rook_semi_open_file = S(10, 11);
		Score trapped_rook = S(-50, -10);

		Score control_space[PIECE_TYPE_NB] = {
			S(0, 0), S(20, 30), S(20, 0), S(10, 5), S(10, 10), S(10, 20), S(0, 0)};
	};

	// Pawn structure bonuses and penalties
	struct PawnStructure
	{
		Score control_center = S(30, 30);
		Score passed = S(20, 40);
		Value passed_rank_weight[RANK_NB] = {0, 1, 1, 2, 3, 6, 10, 0};
		Score doubled = S(-15, -45);
		Value connected_bonus[RANK_NB] = {0, 0, 2, 5, 20, 40, 80, 0};
		Score backward = S(-30, -100);
		Score isolated = S(-20, -80);
		Score pawn_island = S(-10, -20);
		Score same_color_as_bishop = S(-3, -5);
	};

	// King safety related bonuses and penalties
	struct KingSafety
	{
		Score bonus = S(30, 0);
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
	constexpr Material B_MATERIAL;
	constexpr PieceBonus B_PIECE;
	constexpr PawnStructure B_PAWN;
	constexpr KingSafety B_KING;

	// For backward compatibility (can be removed after updating evaluation function)
	constexpr Score PIECE_VALUE[PIECE_TYPE_NB] = {
		B_MATERIAL.piece_value[0], B_MATERIAL.piece_value[1], B_MATERIAL.piece_value[2],
		B_MATERIAL.piece_value[3], B_MATERIAL.piece_value[4], B_MATERIAL.piece_value[5],
		B_MATERIAL.piece_value[6]};

	constexpr Value VALUE_ALL_PIECES = B_MATERIAL.all_pieces();
	constexpr Score MOBILITY_BONUS[PIECE_TYPE_NB] = {
		B_PIECE.mobility[0], B_PIECE.mobility[1], B_PIECE.mobility[2],
		B_PIECE.mobility[3], B_PIECE.mobility[4], B_PIECE.mobility[5],
		B_PIECE.mobility[6]};

	class Position;

	// This namespace aims to improve the evaluation of the position
	// by using a combination of material and positional evaluation
	// The evaluation is done using a combination of material and positional evaluation
	namespace Eval
	{
		Value simple_evaluation(const Position &);

		Value king_pawn(Color c, File file);
		Value piece(Color c, Position &pos, PieceType pt, int &game_phase);
		Value pieces(Position &pos, int &game_phase);

		Value evaluate(const Position &);
	} // namespace Eval
} // namespace KhaosChess
