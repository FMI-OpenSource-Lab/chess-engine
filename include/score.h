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
// Texel-tuned by tools/tuner.cpp (coordinate descent over the zurichess
// quiet-labeled dataset, 725k positions), seeded from the CPW
// simplified-evaluation tables. Pawn back ranks stay zero.
inline Score PSQT[PIECE_TYPE_NB][SQUARE_TOTAL] = {
    {   // NO_PIECE_TYPE
     S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0),
     S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0),
     S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0),
     S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0),
     S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0),
     S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0),
     S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0),
     S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0),
     S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0),
     S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0),
     S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0),
     S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0),
     S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0),
     S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0),
     S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0),
     S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0)},
    {   // PAWN
     S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0),
     S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0),
     S( 294, 381), S( 516, 326), S( 262, 247), S( 562,  78),
     S( 434, 160), S( 662,  69), S( -47, 325), S(-309, 493),
     S(  63, 337), S(  52, 316), S( 165, 208), S( 153,  79),
     S( 290,   2), S( 393,  56), S(  92, 204), S( -10, 256),
     S(  47, 166), S(  94, 145), S(  70,  95), S( 182,  37),
     S( 189,  61), S( 164,  40), S( 109, 116), S(  10,  90),
     S( -27, 130), S( -44, 147), S(  52,  70), S( 112,  77),
     S( 124,  82), S( 129,  28), S(  26,  75), S( -16,  38),
     S(   6,  76), S( -22,  82), S(  39,  26), S(  34,  24),
     S(  66,  25), S( 119,   9), S( 124,  -5), S(  65, -14),
     S( -60, 128), S(  -4,  67), S( -61,  89), S( -81,  62),
     S( -71,  95), S( 102,  31), S( 122,   2), S( -23,  11),
     S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0),
     S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0)},
    {   // KNIGHT
     S(-793, 167), S(-424, 189), S(-197, 274), S(-128, 175),
     S( 344, 142), S(-384, 204), S( -64,  36), S(-450, -35),
     S(-295, 237), S(-160, 291), S( 428,  86), S( 126, 253),
     S( 139, 189), S( 443,  75), S( 104, 195), S(  37,  85),
     S(-171, 223), S( 273, 154), S( 199, 224), S( 244, 199),
     S( 393, 141), S( 652, 114), S( 410, 140), S( 270, 107),
     S(  74, 256), S( 178, 251), S(  93, 326), S( 250, 283),
     S( 101, 311), S( 379, 261), S( 118, 298), S( 238, 246),
     S(  54, 259), S(  85, 229), S( 146, 278), S(  54, 305),
     S( 135, 275), S( 130, 294), S( 140, 300), S(  78, 254),
     S(   1, 231), S(  10, 264), S(  70, 234), S(  79, 288),
     S(  97, 254), S( 126, 206), S( 117, 162), S(  26, 246),
     S( -13, 219), S(-134, 272), S(  40, 302), S(  47, 298),
     S(  64, 304), S( 114, 251), S(  18, 246), S(  35, 180),
     S(-444, 360), S(   1, 171), S(-154, 298), S(-120, 343),
     S( -16, 271), S( -63, 288), S(  -5, 155), S( -20, 110)},
    {   // BISHOP
     S(-132, 209), S(  -4, 209), S(-487, 304), S(-332, 289),
     S(-180, 302), S(-155, 275), S(   3, 227), S( 122, 176),
     S( -44, 251), S( 138, 213), S( -25, 279), S( -58, 224),
     S( 174, 243), S( 399, 195), S( 189, 221), S(-113, 224),
     S( -55, 278), S( 174, 199), S( 250, 173), S( 155, 209),
     S( 193, 198), S( 238, 226), S( 164, 225), S(  30, 298),
     S(  34, 251), S(  70, 240), S(  75, 265), S( 239, 229),
     S( 208, 235), S( 122, 242), S(  88, 226), S(  54, 276),
     S(  29, 227), S(  84, 225), S(  66, 254), S( 152, 254),
     S( 165, 184), S(  63, 226), S(  67, 181), S(  87, 235),
     S(  56, 250), S(  96, 244), S( 113, 267), S(  44, 251),
     S(  58, 259), S( 161, 179), S(  77, 213), S(  87, 200),
     S(  33, 249), S( 168, 191), S(  78, 232), S(  10, 274),
     S(  49, 244), S(  92, 227), S( 222, 142), S(  53, 154),
     S( -96, 253), S(  14, 275), S( -14, 244), S( -62, 272),
     S( -45, 280), S( -67, 242), S(-231, 298), S( -84, 224)},
    {   // ROOK
     S( 269, 215), S( 337, 189), S( 210, 247), S( 411, 185),
     S( 435, 190), S( 264, 219), S( 248, 225), S( 248, 204),
     S( 239, 208), S( 239, 223), S( 390, 184), S( 388, 192),
     S( 482, 123), S( 580, 128), S( 187, 237), S( 322, 188),
     S( 119, 241), S( 180, 240), S( 187, 230), S( 185, 241),
     S( 123, 236), S( 357, 170), S( 517, 135), S( 210, 184),
     S(  65, 234), S(  77, 222), S( 128, 263), S( 188, 201),
     S( 170, 213), S( 322, 196), S( 130, 215), S( 118, 231),
     S(  50, 228), S(  50, 236), S( 103, 243), S( 118, 225),
     S( 184, 186), S( 167, 192), S( 235, 157), S( 101, 183),
     S(  28, 205), S( 108, 199), S( 108, 178), S( 102, 200),
     S( 158, 173), S( 225, 147), S( 198, 155), S( 112, 142),
     S(  49, 200), S( 135, 164), S(  97, 201), S( 155, 208),
     S( 203, 158), S( 264, 138), S( 190, 118), S(  -4, 205),
     S( 125, 185), S( 128, 188), S( 135, 195), S( 155, 175),
     S( 155, 160), S( 169, 149), S(  73, 162), S( 176,  75)},
    {   // QUEEN
     S( 207, 264), S( 271, 406), S( 367, 326), S( 406, 284),
     S( 945,  44), S( 896,  87), S( 577, 218), S( 573, 375),
     S( 282, 208), S( 195, 265), S( 333, 267), S( 349, 354),
     S( 126, 542), S( 684, 186), S( 512, 269), S( 682, 238),
     S( 407, 196), S( 341, 232), S( 479,  87), S( 272, 459),
     S( 463, 393), S( 660, 289), S( 527, 347), S( 579, 359),
     S( 256, 434), S( 263, 389), S( 283, 338), S( 267, 378),
     S( 342, 448), S( 404, 384), S( 346, 587), S( 364, 584),
     S( 400, 358), S( 224, 623), S( 356, 459), S( 355, 570),
     S( 359, 492), S( 398, 521), S( 381, 610), S( 410, 564),
     S( 347, 488), S( 425, 355), S( 374, 545), S( 416, 440),
     S( 375, 551), S( 417, 587), S( 420, 624), S( 434, 594),
     S( 235, 548), S( 380, 471), S( 465, 423), S( 436, 510),
     S( 450, 533), S( 458, 533), S( 405, 386), S( 481, 451),
     S( 413, 450), S( 360, 412), S( 392, 480), S( 444, 469),
     S( 338, 603), S( 289, 532), S( 321, 499), S( 169, 465)},
    {   // KING
     S(-415,-525), S( 616,-396), S( 943,-356), S( 804,-375),
     S(-374,-106), S(-145, -22), S( 284,-174), S( 345,-397),
     S( 954,-368), S( 257,  32), S( 163,  30), S( 368,   2),
     S( 190,  40), S( 324, 116), S(-105, 118), S(-610,  26),
     S( 349,-153), S( 263,  57), S( 416,  28), S( 143,  38),
     S( 391,   5), S( 563, 108), S( 436, 120), S(-117, -63),
     S(  54,-186), S( -82, 102), S(  79,  81), S( -25, 106),
     S( -69, 104), S( -82, 131), S(-127, 125), S(-386, -61),
     S(-227,-187), S(   5, -25), S(-242, 100), S(-374, 141),
     S(-442, 161), S(-312, 106), S(-350,  67), S(-344,-137),
     S( -75,-202), S(-145,  -1), S(-237,  73), S(-324, 111),
     S(-369, 133), S(-337, 100), S(-192,  44), S(-143,-158),
     S(  40,-261), S( -30, -53), S(-182,  48), S(-403,  95),
     S(-338,  99), S(-246,  67), S( -45, -31), S( 103,-247),
     S(  99,-428), S( 224,-293), S(  95,-192), S(-124,-119),
     S( -37,-170), S( -52,-134), S( 183,-236), S( 300,-431)}};

// clang-format on

// Material values
struct Material {
    Score piece_value[PIECE_TYPE_NB] = {S(0, 0), S(416, 409), S(1546, 956),
                                        S(1608, 1113), S(2195, 2151),
                                        S(4905, 3508), S(0, 0)};

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
    Score mobility[PIECE_TYPE_NB] = {S(0, 0), S(5, 10), S(6, 24), S(22, 23),
                                     S(8, 22), S(4, 20), S(58, -51)};

    Score bishop_pair = S(146, 192);
    Score connected_rooks = S(57, 82);
    Score outpost_knight = S(41, 109);
    Score outpost_bishop = S(19, 100);
    Score safe_knight = S(-20, -60);
    Score control_center_knight = S(25, 10);
    Score vulnerable_queen = S(-104, 87);

    Score rook_open_file = S(278, -70);
    Score rook_semi_open_file = S(82, 47);

    Score control_space[PIECE_TYPE_NB] = {
        S(0, 0), S(20, 30), S(11, 15), S(3, 3), S(0, 3), S(-5, 48), S(0, 0)};

    // Penalties
    Score trapped_rook = S(-97, 6);

    Score pinned_penalty[PIECE_TYPE_NB] = {S(0, 0), S(-20, -20), S(-10, -15),
                                           S(-10, -20), S(0, 0), S(0, 0),
                                           S(0, 0)};
};

// Pawn structure bonuses and penalties
struct PawnStructure {
    // Bonuses
    Value passed_rank_weight[RANK_NB] = {0, 29, 16, 9, 3, 1, 0, 0};
    Value connected_bonus[RANK_NB] = {0, 320, 160, 56, 35, 2, 5, 0};

    Score control_center = S(12, -44);
    Score passed = S(8, 27);

    // Penalties
    Score doubled = S(-47, -116);
    Score backward = S(-105, 0);
    Score isolated = S(-86, -17);
    Score pawn_island = S(266, 65);
    Score same_color_as_bishop = S(1, -30);
};

// King safety related bonuses and penalties
struct KingSafety {
    Score bonus = S(65, -29);

    // Penalties
    Score weak_diagonals = S(-32, -8);
    Score weak_lines = S(-27, -8);
    Score weak_back_rank = S(-7, -198);
    Score pawn_proximity_penalty = S(-14, -7);

    Score protector_penalty[PIECE_TYPE_NB] = {
        S(0, 0), S(0, 0), S(-15, 2), S(-18, 1), S(0, 0), S(0, 0), S(0, 0)};

    Score attacker_penalty[PIECE_TYPE_NB] = {
        S(0, 0), S(0, 0), S(19, -4), S(24, -5), S(0, 0), S(0, 0), S(0, 0)};
};

// Define the actual instances
inline Material MATERIAL_SCORES;
inline PieceBonus PIECE_SCORES;
inline PawnStructure PAWN_STRUCTURE_SCORES;
inline KingSafety KING_SAFETY_SCORES;

inline Value TEMPO = 93;

const Value PIECE_WEIGHTS[PIECE_TYPE_NB] = {0, 0, 1, 1, 2, 4, 0};

// 2 * ((2 * knights + 2 * bishops + 2 * rooks) + 1 * queen)
// double the number of knights, bishops, rooks and queens for a side
const Value MAX_PIECE_WEIGHTS =
    4 * (PIECE_WEIGHTS[KNIGHT] + PIECE_WEIGHTS[BISHOP] + PIECE_WEIGHTS[ROOK]) +
    2 * PIECE_WEIGHTS[QUEEN];

constexpr Value VALUE_INFINITE = 640'001;
constexpr Value VALUE_MATE = 640'000;
constexpr Value VALUE_KNOWN_WIN =
    640'001 - 8 * MAX_PLY - 8 * Material().all_pieces();

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
