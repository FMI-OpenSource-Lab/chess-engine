#pragma once

#include <array>
#include <iomanip>

#include "consts.h"
#include "defs.h"
#include "endgame.h"
#include "movegen.h"
#include "position.h"

namespace KhaosChess {
struct Score {
    Value mg;  // Middlegame score
    Value eg;  // Endgame score

    constexpr Score() : mg(0), eg(0) {
    }
    constexpr Score(Value mg, Value eg) : mg(mg), eg(eg) {
    }
    constexpr Score(Value v) : mg(v), eg(v) {
    }
    Score(const Score& other) = default;
    Score(Score&& other) = default;

    constexpr Score& operator=(const Score& other) = default;
    constexpr Score& operator=(Score&& other) = default;

    // Addition operators
    constexpr Score operator+(const Score& other) const {
        return Score(mg + other.mg, eg + other.eg);
    }
    constexpr Score& operator+=(const Score& other) {
        mg += other.mg;
        eg += other.eg;
        return *this;
    }

    // Subtraction operators
    constexpr Score operator-(const Score& other) const {
        return Score(mg - other.mg, eg - other.eg);
    }
    constexpr Score operator-() const {
        return Score(-mg, -eg);
    }
    constexpr Score& operator-=(const Score& other) {
        mg -= other.mg;
        eg -= other.eg;
        return *this;
    }

    // Multiplication operators
    constexpr Score operator*(const Score& other) const {
        return Score(mg * other.mg, eg * other.eg);
    }

    // Division operators
    constexpr Score operator/(const Score& other) const {
        return Score(mg / other.mg, eg / other.eg);
    }
};

constexpr Score operator*(const Value& value, const Score& other) {
    return Score(other.mg * value, other.eg * value);
}

inline std::ostream& operator<<(std::ostream& os, Score s) {
    return os << std::setw(6) << std::setfill(' ') << s.mg << " " << std::setw(6)
              << std::setfill(' ') << s.eg << " ";
}

constexpr Score S(Value mg, Value eg) {
    return Score(mg, eg);
}

// clang-format off

// Piece-square table, indexed [PieceType][sq_relative_to_side(s, Us)]
// (identity for White, vertical flip for Black). Written from White's
// perspective: first row = rank 8, matching Square A8 = 0.
// Plain numbers use Score's mg = eg constructor; the king gets explicit
// S(mg, eg) pairs since its placement inverts between game phases.
// Seeded from the CPW simplified-evaluation tables scaled to KhaosChess
// units (pawn mg = 300); placeholder values until Texel tuning.
constexpr Score PSQT[PIECE_TYPE_NB][SQUARE_TOTAL] = {
    {   // NO_PIECE_TYPE
        0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0},
    {   // PAWN (mg = eg until Texel tuning)
        0,    0,    0,    0,    0,    0,    0,    0,
      150,  150,  150,  150,  150,  150,  150,  150,
       30,   30,   60,   90,   90,   60,   30,   30,
       15,   15,   30,   75,   75,   30,   15,   15,
        0,    0,    0,   60,   60,    0,    0,    0,
       15,  -15,  -30,    0,    0,  -30,  -15,   15,
       15,   30,   30,  -60,  -60,   30,   30,   15,
        0,    0,    0,    0,    0,    0,    0,    0},
    {   // KNIGHT (mg = eg until Texel tuning)
     -150, -120,  -90,  -90,  -90,  -90, -120, -150,
     -120,  -60,    0,    0,    0,    0,  -60, -120,
      -90,    0,   30,   45,   45,   30,    0,  -90,
      -90,   15,   45,   60,   60,   45,   15,  -90,
      -90,    0,   45,   60,   60,   45,    0,  -90,
      -90,   15,   30,   45,   45,   30,   15,  -90,
     -120,  -60,    0,   15,   15,    0,  -60, -120,
     -150, -120,  -90,  -90,  -90,  -90, -120, -150},
    {   // BISHOP (mg = eg until Texel tuning)
      -60,  -30,  -30,  -30,  -30,  -30,  -30,  -60,
      -30,    0,    0,    0,    0,    0,    0,  -30,
      -30,    0,   15,   30,   30,   15,    0,  -30,
      -30,   15,   15,   30,   30,   15,   15,  -30,
      -30,    0,   30,   30,   30,   30,    0,  -30,
      -30,   30,   30,   30,   30,   30,   30,  -30,
      -30,   15,    0,    0,    0,    0,   15,  -30,
      -60,  -30,  -30,  -30,  -30,  -30,  -30,  -60},
    {   // ROOK (mg = eg until Texel tuning)
        0,    0,    0,    0,    0,    0,    0,    0,
       15,   30,   30,   30,   30,   30,   30,   15,
      -15,    0,    0,    0,    0,    0,    0,  -15,
      -15,    0,    0,    0,    0,    0,    0,  -15,
      -15,    0,    0,    0,    0,    0,    0,  -15,
      -15,    0,    0,    0,    0,    0,    0,  -15,
      -15,    0,    0,    0,    0,    0,    0,  -15,
        0,    0,    0,   15,   15,    0,    0,    0},
    {   // QUEEN (mg = eg until Texel tuning)
      -60,  -30,  -30,  -15,  -15,  -30,  -30,  -60,
      -30,    0,    0,    0,    0,    0,    0,  -30,
      -30,    0,   15,   15,   15,   15,    0,  -30,
      -15,    0,   15,   15,   15,   15,    0,  -15,
        0,    0,   15,   15,   15,   15,    0,  -15,
      -30,   15,   15,   15,   15,   15,    0,  -30,
      -30,    0,   15,    0,    0,    0,    0,  -30,
      -60,  -30,  -30,  -15,  -15,  -30,  -30,  -60},
    {   // KING
     S( -90,-150), S(-120,-120), S(-120, -90), S(-150, -60),
     S(-150, -60), S(-120, -90), S(-120,-120), S( -90,-150),
     S( -90, -90), S(-120, -60), S(-120, -30), S(-150,   0),
     S(-150,   0), S(-120, -30), S(-120, -60), S( -90, -90),
     S( -90, -90), S(-120, -30), S(-120,  60), S(-150,  90),
     S(-150,  90), S(-120,  60), S(-120, -30), S( -90, -90),
     S( -90, -90), S(-120, -30), S(-120,  90), S(-150, 120),
     S(-150, 120), S(-120,  90), S(-120, -30), S( -90, -90),
     S( -60, -90), S( -90, -30), S( -90,  90), S(-120, 120),
     S(-120, 120), S( -90,  90), S( -90, -30), S( -60, -90),
     S( -30, -90), S( -60, -30), S( -60,  60), S( -60,  90),
     S( -60,  90), S( -60,  60), S( -60, -30), S( -30, -90),
     S(  60, -90), S(  60, -90), S(   0,   0), S(   0,   0),
     S(   0,   0), S(   0,   0), S(  60, -90), S(  60, -90),
     S(  60,-150), S(  90, -90), S(  30, -90), S(   0, -90),
     S(   0, -90), S(  30, -90), S(  90, -90), S(  60,-150)}};

// clang-format on

// Material values
struct Material {
    Score piece_value[PIECE_TYPE_NB] = {S(0, 0), S(300, 370), S(890, 880),
                                        S(900, 950), S(1400, 1550), S(2900, 2800),
                                        S(0, 0)};

    constexpr Value all_pieces() const {
        return 2 * (8 * piece_value[PAWN] +    // 8 PAWNS
                    2 * piece_value[KNIGHT] +  // 2 KNIGHTS
                    2 * piece_value[BISHOP] +  // 2 BISHOPS
                    2 * piece_value[ROOK] +    // 2 ROOKS
                    1 * piece_value[QUEEN])
                       .eg;  // 1 QUEEN
    }
};

// Piece-specific bonuses and penalties
struct PieceBonus {
    // Bonuses
    Score mobility[PIECE_TYPE_NB] = {S(0, 0), S(5, 10), S(12, 24), S(18, 8),
                                     S(6, 24), S(4, 12), S(0, 10)};

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

    Score pinned_penalty[PIECE_TYPE_NB] = {S(0, 0), S(-20, -20), S(-10, -15),
                                           S(-10, -20), S(0, 0), S(0, 0),
                                           S(0, 0)};
};

// Pawn structure bonuses and penalties
struct PawnStructure {
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
struct KingSafety {
    Score bonus = S(30, 0);

    // Penalties
    Score weak_diagonals = S(-5, 0);
    Score weak_lines = S(-7, 0);
    Score weak_back_rank = S(-75, -100);
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

const Value PIECE_WEIGHTS[PIECE_TYPE_NB] = {0, 0, 1, 1, 2, 4, 0};

// 2 * ((2 * knights + 2 * bishops + 2 * rooks) + 1 * queen)
// double the number of knights, bishops, rooks and queens for a side
const Value MAX_PIECE_WEIGHTS =
    4 * (PIECE_WEIGHTS[KNIGHT] + PIECE_WEIGHTS[BISHOP] + PIECE_WEIGHTS[ROOK]) +
    2 * PIECE_WEIGHTS[QUEEN];

constexpr Value VALUE_INFINITE = 640'001;
constexpr Value VALUE_MATE = 640'000;
constexpr Value VALUE_KNOWN_WIN =
    640'001 - 8 * MAX_PLY - 8 * MATERIAL_SCORES.all_pieces();
constexpr Value TEMPO = 20;

// Kill-switches for the tempo + PSQT batch
constexpr bool TEMPO_ENABLED = true;
constexpr bool PSQT_ENABLED = true;

class Position;

enum ScoreComponent : uint8_t {
    SC_MATERIAL,            // The material scores
    SC_MOBILITY,            // The mobility scores
    SC_KING_SAFETY,         // The king safety scores
    SC_PAWN_STRUCTURE,      // The pawn structure scores
    SC_PIECE_COORDINATION,  // The piece coordination scores
    SC_ALL                  // Combined score
};

template <ScoreComponent>
Score total_scores(const Position& pos);

template <ScoreComponent T>
struct Scorer {
    explicit Scorer() : score(0), weight(0) {};

    Value get_score(const Position& pos);
    Value get_weight() {
        return weight;
    }

    void print_stats(const Position& pos);

   private:
    Score score;
    Value weight;

    Value game_phase_weights(const Position& pos) {
        Value w = 0;

        w += (pos.count<KNIGHT>(WHITE) + pos.count<KNIGHT>(BLACK)) *
             PIECE_WEIGHTS[KNIGHT];
        w += (pos.count<BISHOP>(WHITE) + pos.count<BISHOP>(BLACK)) *
             PIECE_WEIGHTS[BISHOP];
        w +=
            (pos.count<ROOK>(WHITE) + pos.count<ROOK>(BLACK)) * PIECE_WEIGHTS[ROOK];
        w += (pos.count<QUEEN>(WHITE) + pos.count<QUEEN>(BLACK)) *
             PIECE_WEIGHTS[QUEEN];

        return std::min(w, MAX_PIECE_WEIGHTS);
    }

    Value combine(const Score& score, const Value& weight);
};

template <ScoreComponent T>
inline Value Scorer<T>::combine(const Score& score, const Value& weight) {
    assert(0 <= weight && weight <= MAX_PIECE_WEIGHTS);
    return (score.mg * weight + score.eg * (MAX_PIECE_WEIGHTS - weight)) /
           MAX_PIECE_WEIGHTS;
}
}  // namespace KhaosChess
