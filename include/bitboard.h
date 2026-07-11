#pragma once

#include <array>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>

#include "consts.h"
#include "defs.h"

namespace KhaosChess {
constexpr std::uint16_t BISHOP_ATTACKS_SIZE = 512;
constexpr std::int16_t ROOK_ATTACKS_SIZE = 4096;

// extern void init_sliders_attacks(PieceType py);
extern void init_pseudo_attacks();

extern BITBOARD bishopAttacks(BITBOARD occ, Square s);
extern BITBOARD rookAttacks(BITBOARD occ, Square s);

constexpr BITBOARD square_to_BB(Square square) {
    assert(is_square_ok(square));
    return (1ULL << square);
}

namespace Bitboards {
void init();
};

// overloads of bitwise operators between bitboard and square for testing
// purposes
inline BITBOARD operator&(BITBOARD b, Square s) {
    return b & square_to_BB(s);
}
inline BITBOARD operator|(BITBOARD b, Square s) {
    return b | square_to_BB(s);
}
inline BITBOARD operator^(BITBOARD b, Square s) {
    return b ^ square_to_BB(s);
}
inline BITBOARD& operator|=(BITBOARD& b, Square s) {
    return b |= square_to_BB(s);
}
inline BITBOARD& operator^=(BITBOARD& b, Square s) {
    return b ^= square_to_BB(s);
}

inline BITBOARD operator&(Square s, BITBOARD b) {
    return b & s;
}
inline BITBOARD operator|(Square s, BITBOARD b) {
    return b | s;
}
inline BITBOARD operator^(Square s, BITBOARD b) {
    return b ^ s;
}

inline BITBOARD operator|(Square s1, Square s2) {
    return square_to_BB(s1) | s2;
}

// checks for available bit
inline BITBOARD get_bit(BITBOARD bitboard, Square square) {
    return bitboard & square;
}

// set piece to square
inline void set_bit(BITBOARD& bitboard, Square square) {
    bitboard |= square;
}

// if theres a 1 remove it, if 0 don't
inline void rm_bit(BITBOARD& bitboard, Square square) {
    bitboard &= ~square_to_BB(square);
}

// Files
constexpr BITBOARD FileA_Bits = 0x0101010101010101ULL;  // first row is ones
constexpr BITBOARD FileB_Bits = FileA_Bits << 1;
constexpr BITBOARD FileC_Bits = FileA_Bits << 2;
constexpr BITBOARD FileD_Bits = FileA_Bits << 3;
constexpr BITBOARD FileE_Bits = FileA_Bits << 4;
constexpr BITBOARD FileF_Bits = FileA_Bits << 5;
constexpr BITBOARD FileG_Bits = FileA_Bits << 6;
constexpr BITBOARD FileH_Bits = 0x8080808080808080ULL;

// Ranks
constexpr BITBOARD Rank8_Bits = 0xFF;
constexpr BITBOARD Rank7_Bits = Rank8_Bits << (8 * 1);
constexpr BITBOARD Rank6_Bits = Rank8_Bits << (8 * 2);
constexpr BITBOARD Rank5_Bits = Rank8_Bits << (8 * 3);
constexpr BITBOARD Rank4_Bits = Rank8_Bits << (8 * 4);
constexpr BITBOARD Rank3_Bits = Rank8_Bits << (8 * 5);
constexpr BITBOARD Rank2_Bits = Rank8_Bits << (8 * 6);
constexpr BITBOARD Rank1_Bits = Rank8_Bits << (8 * 7);

// Diagonals
constexpr BITBOARD Diagonal_H1_A8 = 0x8040201008040201ULL;
constexpr BITBOARD Diagonal_A1_H8 = 0x0102040810204080ULL;

constexpr BITBOARD DarkSquares = 0x55AA55AA55AA55AAULL;
constexpr BITBOARD LightSquares = 0xAA55AA55AA55AA55ULL;

constexpr BITBOARD rank_bb(Square s) {
    return Rank8_Bits << (rank_of(s) << 3);
}
constexpr BITBOARD rank_bb(Rank r) {
    return Rank8_Bits << (8 * r);
}
constexpr BITBOARD file_bb(Square s) {
    return FileA_Bits << file_of(s);
}
constexpr BITBOARD file_bb(File f) {
    return FileA_Bits << f;
}

constexpr BITBOARD board_edges(Square s) {
    return ((Rank8_Bits | Rank1_Bits) & ~rank_bb(s)) |
           ((FileA_Bits | FileH_Bits) & ~file_bb(s));
}

// Not files
constexpr BITBOARD not_A =
    18374403900871474942ULL;  // ~FileA_Bits bitboard value where the A file
                              // is set to zero
constexpr BITBOARD not_H =
    9187201950435737471ULL;  // ~FileH_Bits bitboard value where the H file
                             // is set to zero
constexpr BITBOARD not_HG =
    4557430888798830399ULL;  // ~FileH_Bits & ~FileG_Bits bitboard value
                             // where the HG files are set to zero
constexpr BITBOARD not_AB =
    18229723555195321596ULL;  // ~FileA_Bits & ~FileB_Bits bitboard value
                              // where the HG files are set to zero

constexpr BITBOARD neighboring_files[FILE_NB] = {
    FileB_Bits,               // FILE_A
    FileA_Bits | FileC_Bits,  // FILE_B
    FileB_Bits | FileD_Bits,  // FILE_C
    FileC_Bits | FileE_Bits,  // FILE_D
    FileD_Bits | FileF_Bits,  // FILE_E
    FileE_Bits | FileG_Bits,  // FILE_F
    FileF_Bits | FileH_Bits,  // FILE_G
    FileG_Bits,               // FILE_H
};

//// define magic bishop attack table [squares][occupancy]
extern BITBOARD mBishopAttacks[SQUARE_TOTAL][BISHOP_ATTACKS_SIZE];
//// define magic rook attack table [squares][occupancy]
extern BITBOARD mRookAttacks[SQUARE_TOTAL][ROOK_ATTACKS_SIZE];

// every pseudo attack for the given piece on the given square
extern BITBOARD pseudo_attacks[PIECE_TYPE_NB][SQUARE_TOTAL];

// pawn attacks table [side][square]
extern BITBOARD pawn_attacks[2]
                            [64];  // 2 - sides to play, 64 - squares on a table

// distance in squares between square x and square y
extern std::int32_t square_distance[SQUARE_TOTAL][SQUARE_TOTAL];

// squares between 2 points
extern BITBOARD between_points_bb[SQUARE_TOTAL][SQUARE_TOTAL];

// line between 2 points from edge to edge
extern BITBOARD full_line_bb[SQUARE_TOTAL][SQUARE_TOTAL];

// extern U64 bishop_attacks[];
// extern U64 rook_attacks[];

struct SMagic {
    BITBOARD* attack;
    BITBOARD mask;       // to mask relevant squares of both lines (no outer squares)
    BITBOARD magic;      // magic 64-bit factor
    std::int32_t shift;  // shift relevant bits
};

extern SMagic bishop_magic_tbl[SQUARE_TOTAL];
extern SMagic rook_magic_tbl[SQUARE_TOTAL];

extern void init_magics(PieceType pt, SMagic magics[]);
extern BITBOARD sliding_attacks(PieceType pt, Square s, BITBOARD occ);

// distance functions return the distance between x and y
template <typename T1 = Square>
inline std::int32_t distance(Square x, Square z);

template <>
inline std::int32_t distance<Rank>(
    Square x,
    Square y)  // The distance between two squares in the same rank
{
    return std::abs(rank_of(x) - rank_of(y));
}

template <>
inline std::int32_t distance<File>(
    Square x,
    Square y)  // The distance between two squares in the same file
{
    return std::abs(file_of(x) - file_of(y));
}

template <>
inline std::int32_t distance<Square>(
    Square x,
    Square y)  // The distance between two squares
{
    return square_distance[x][y];
}

/// @brief Distance to the nearest edge of the board, order matters
/// @param f given file
/// @return minimum of given file and difference between the given file and the
/// last file
inline std::int32_t nearest_edge_distance(File f) {
    return std::min(f, File(FILE_H - f));
}

/// @brief Distance to the nearest edge of the board, order matters
/// @param r given rank
/// @return minimum of given rank and difference between the given rank and the
/// last rank
inline std::int32_t nearest_edge_distance(Rank r) {
    return std::min(r, Rank(RANK_1 - r));
}

/// @brief Distance between two files, order does not matter
/// @param f1 start file
/// @param f2 end file
/// @return Difference between the two files
inline std::int32_t edge_distance(File f1, File f2) {
    return std::abs(f1 - f2);
}

/// @brief Distance between two ranks, order does not matter
/// @param r1 start rank
/// @param r2 end rank
/// @return Difference between the two ranks
inline std::int32_t edge_distance(Rank r1, Rank r2) {
    return std::abs(r1 - r2);
}

// moves the bb one or two steps
template <Direction d>
constexpr BITBOARD move_to(BITBOARD b) {
    return d == DOWN          ? b << 8
           : d == UP          ? b >> 8
           : d == DOWN + DOWN ? b << 16
           : d == UP + UP     ? b >> 16
           : d == LEFT        ? (b & not_A) >> 1
           : d == RIGHT       ? (b & not_H) << 1
           : d == DOWN_LEFT   ? (b & not_A) << 7
           : d == DOWN_RIGHT  ? (b & not_H) << 9
           : d == UP_LEFT     ? (b & not_A) >> 9
           : d == UP_RIGHT    ? (b & not_H) >> 7
                              : 0;
}

template <Color c>
constexpr BITBOARD pawn_attacks_bb(BITBOARD bb) {
    return c == WHITE ? move_to<UP_RIGHT>(bb) | move_to<UP_LEFT>(bb)
                      : move_to<DOWN_RIGHT>(bb) | move_to<DOWN_LEFT>(bb);
}

constexpr BITBOARD pawn_attacks_bb(Color c, BITBOARD bb) {
    return c == WHITE ? pawn_attacks_bb<WHITE>(bb) : pawn_attacks_bb<BLACK>(bb);
}

constexpr BITBOARD pawn_attacks_bb(Color c, Square bb) {
    return c == WHITE ? pawn_attacks_bb<WHITE>(square_to_BB(bb))
                      : pawn_attacks_bb<BLACK>(square_to_BB(bb));
}

template <PieceType pt>
inline BITBOARD attacks_bb_by(Square s, BITBOARD occ) {
    assert(is_square_ok(s) && pt != PAWN);

    switch (pt) {
        case BISHOP:
            return bishopAttacks(occ, s);
        case ROOK:
            return rookAttacks(occ, s);
        case QUEEN:
            return bishopAttacks(occ, s) | rookAttacks(occ, s);
        default:  // king and knight
            return pseudo_attacks[pt][s];
    }
}

inline BITBOARD attacks_bb_by(PieceType pt, Square s, BITBOARD occ) {
    switch (pt) {
        case BISHOP:
            return attacks_bb_by<BISHOP>(s, occ);
        case ROOK:
            return attacks_bb_by<ROOK>(s, occ);
        case QUEEN:
            return attacks_bb_by<QUEEN>(s, occ);
        default:  // king and knight
            return pseudo_attacks[pt][s];
    }
}

// For easier access to knight and king attacks
template <PieceType pt>
inline BITBOARD attacks_bb_by(Square s) {
    assert(is_square_ok(s) && (pt != PAWN));

    return pseudo_attacks[pt][s];
}

// Compiler specific functions, taken from Stockfish
// https://github.com/official-stockfish/Stockfish GCC, Clang, ICC
constexpr Square get_ls1b(BITBOARD bitboard) {
    if (!bitboard)
        return NONE;

#if defined(__GNUC__)
    return Square(__builtin_ctzll(bitboard));

#elif defined(_MSC_VER)  // MSVC

#ifdef _WIN64

    std::std::uint32_t idx;
    _BitScanForward64(&idx, bitboard);
    return Square(idx);

#else  // MSVC, WIN32

    std::std::uint32_t idx;

    if (bitboard & 0xffffffff) {
        _BitScanForward(&idx, int32_t(bitboard));
        return Square(idx);
    } else {
        _BitScanForward(&idx, int32_t(bitboard >> 32));
        return Square(idx + 32);
    }

#endif

#else  // Compiler is neither GCC nor MSVC compatible

#error "Compiler not supported."

#endif
}

constexpr Square get_msb(BITBOARD bitboard) {
    if (!bitboard)
        return NONE;

#if defined(__GNUC__)  // GCC, Clang, ICX

    return Square(63 ^ __builtin_clzll(bitboard));

#elif defined(_MSC_VER)

#ifdef _WIN64  // MSVC, WIN64

    std::std::uint32_t idx;
    _BitScanReverse64(&idx, bitboard);
    return Square(idx);

#else  // MSVC, WIN32

    std::std::uint32_t idx;

    if (bitboard >> 32) {
        _BitScanReverse(&idx, int32_t(bitboard >> 32));
        return Square(idx + 32);
    } else {
        _BitScanReverse(&idx, int32_t(bitboard));
        return Square(idx);
    }
#endif

#else  // Compiler is neither GCC nor MSVC compatible

#error "Compiler not supported."

#endif
}

// calculate the line that is formed between two points
// return that line bitboard + the target square
// or only the target square in case of knight attack
inline BITBOARD in_between_bb(Square source, Square target) {
    assert(is_square_ok(source) && is_square_ok(target));
    return between_points_bb[source][target];
}

inline BITBOARD line_bb(Square x, Square y) {
    assert(is_square_ok(x) && is_square_ok(y));
    return full_line_bb[x][y];
}

// Returns true if all 3 squares are aligned
inline bool are_squares_aligned(Square source, Square target,
                                Square aligned_with) {
    return line_bb(source, target) & aligned_with;
}

template <Color c>
constexpr BITBOARD forward_ranks_bb(Square source) {
    assert(is_square_ok(source));
    return c == WHITE ? ~Rank1_Bits >> 8 * (RANK_1 - rank_of(source))
                      : ~Rank8_Bits << 8 * (rank_of(source) - RANK_8);
}

template <Color c>
constexpr BITBOARD passed_pawn_path(Square source) {
    assert(is_square_ok(source));
    return (forward_ranks_bb<c>(source) & neighboring_files[file_of(source)]) |
           file_bb(source);
}

inline void print_bitboard(BITBOARD bitboard) {
    std::cout << "\nBitboard representation:\n\n";

    for (Rank rank = RANK_8; rank <= RANK_1; ++rank) {
        for (File file = FILE_A; file <= FILE_H; ++file) {
            Square sq = make_square(file, rank);
            std::string s = get_bit(bitboard, sq) ? " 1" : " 0";

            std::cout << s;
        }

        std::cout << "  " << (8 - rank) << " \n";
    }

    // print files
    std::cout << "\n a b c d e f g h \n\n";

    // print as udec value
    std::cout << "\n Bitboard: " << bitboard << "\n\n";
}

constexpr bool has_bit_after_pop(BITBOARD bitboard) {
    return bitboard & (bitboard - 1);
}

constexpr Square pop_ls1b(BITBOARD& bitboard) {
    assert(bitboard);

    Square s = get_ls1b(bitboard);
    bitboard &= bitboard - 1;

    return s;
}

const std::uint64_t m1 = 0x5555555555555555;  // binary: 0101...
const std::uint64_t m2 = 0x3333333333333333;  // binary: 00110011..
const std::uint64_t m4 = 0x0f0f0f0f0f0f0f0f;  // binary:  4 zeros,  4 ones ...
const std::uint64_t h01 =
    0x0101010101010101;  // the sum of 256 to the power of 0,1,2,3...

// Hamming weight algorithm for finding the count of the 1's
constexpr std::int32_t count_bits(BITBOARD bitboard) {
    // put count of each 2 bits into those 2 bits
    bitboard -= (bitboard >> 1) & m1;
    // put count of each 4 bits into those 4 bits
    bitboard = (bitboard & m2) + ((bitboard >> 2) & m2);
    // put count of each 8 bits into those 8 bits
    bitboard = (bitboard + (bitboard >> 4)) & m4;

    // returns left 8 bits of x + (x<<8) + (x<<16) + (x<<24) + ...
    return (bitboard * h01) >> 56;
}
}  // namespace KhaosChess
