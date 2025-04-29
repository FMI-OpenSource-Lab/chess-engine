#include "endgame.h"

#include <numeric>

namespace KhaosChess
{
    namespace Endgame
    {
        enum EndgameType : uint8_t
        {
            ET_KPK,    // King and pawn vs king
            ET_KPsK,   // King and multiple pawns vs king
            ET_KNBK,   // King, knight and bishop vs king
            ET_KXK,    // King and any material vs king
            ET_KQKR,   // King and queen vs king and rook
            ET_KRNKR,  // King, rook and knight vs king and rook
            ET_KRBKR,  // King, rook and bishop vs king and rook
            ET_KBPsK,  // King, bishop and pawns vs king
            ET_KQKP,   // King and queen vs king and pawn
            ET_KRKP,   // King and rook vs king and pawn
            ET_KNNK,   // King and 2 knights vs king (theoretical draw)
            ET_KNNKP,  // King and 2 knights vs king and pawn
            ET_KBPsKB, // King, bishop and pawns vs king and bishop
            ET_KRKB,   // King and rook vs king and bishop
            ET_KRKN,   // King and rook vs king and knight
            ET_KQKRPs, // King and queen vs king and rook and pawns
            ET_KmmKm,  // King and multiple minor pieces vs king
        };

        namespace
        {
            // clang-format off

            // Weights to push weak king to the edges and corners
            std::array<Value, SQUARE_TOTAL> PUSH_TO_EDGE_BONUS =
            {
                100, 90, 80, 70, 70, 80, 90, 100,
                90, 60, 50, 40, 40, 50, 60,  90,
                80, 50, 30, 20, 20, 30, 40,  80,
                70, 40, 20, 10, 10, 20, 40,  70,
                70, 40, 20, 10, 10, 20, 40,  70,
                80, 50, 30, 20, 20, 30, 40,  80,
                90, 60, 50, 40, 40, 50, 60,  90,
               100, 90, 80, 70, 70, 80, 90, 100,
            };

             // Weights to push the weak king to the corner of the correct color
             // For default pushes to black corners for white
             // board needs to be flipped horizontally
             std::array<Value, SQUARE_TOTAL> PUSH_TO_COLOR_CORNER_BONUS =
             {
                 100, 90, 80, 70, 70, 80, 90, 100,
                 90, 60, 50, 40, 40, 50, 60,  90,
                 80, 50, 30, 20, 20, 30, 40,  80,
                 70, 40, 20, 10, 10, 20, 40,  70,
                 70, 40, 20, 10, 10, 20, 40,  70,
                 80, 50, 30, 20, 20, 30, 40,  80,
                 90, 60, 50, 40, 40, 50, 60,  90,
                100, 90, 80, 70, 70, 80, 90, 100,
             };

            // clang-format on

            // Weights to have both kings close to each other
            std::array<int, RANK_NB> PUSH_CLOSER = {0, 7, 6, 5, 4, 3, 2, 1};

            // Bitboard when represented bitwaise is displayed revered
            // Because of this the first pawn is always the last significant bit set in the bitboard
            // and the last pawn is always the most significant bit set in the bitboard
            Square first_pawn_square(BITBOARD pawns, Color side) { return (side == WHITE) ? get_ls1b(pawns) : get_msb(pawns); }

            template <EndgameType ET>
            struct SandboxPCV
            {
            };

            // King and pawn vs king
            template <>
            struct SandboxPCV<ET_KPK>
            {
                static constexpr PCV pcv[BOTH] =
                    {
                        encode_pcv(1, 0, 0, 0, 0, 0, 0, 0, 0, 0), // for WHITE
                        encode_pcv(0, 0, 0, 0, 0, 1, 0, 0, 0, 0)  // for BLACK
                    };
            };

            // King, knight and bishop vs king
            template <>
            struct SandboxPCV<ET_KNBK>
            {
                static constexpr PCV pcv[BOTH] =
                    {
                        encode_pcv(0, 1, 1, 0, 0, 0, 0, 0, 0, 0), // for WHITE
                        encode_pcv(0, 0, 0, 0, 0, 0, 1, 1, 0, 0)  // for BLACK
                    };
            };

            // King, knight and bishop vs king
            template <>
            struct SandboxPCV<ET_KQKR>
            {
                static constexpr PCV pcv[BOTH] =
                    {
                        encode_pcv(0, 0, 0, 0, 1, 0, 0, 0, 1, 0), // for WHITE
                        encode_pcv(0, 0, 0, 1, 0, 0, 0, 0, 0, 1)  // for BLACK
                    };
            };

            template <>
            struct SandboxPCV<ET_KRNKR>
            {
                static constexpr PCV pcv[BOTH] = {
                    encode_pcv(0, 1, 0, 1, 0, 0, 0, 0, 1, 0),
                    encode_pcv(0, 0, 0, 1, 0, 0, 1, 0, 1, 0)};
            };

            template <>
            struct SandboxPCV<ET_KRBKR>
            {
                static constexpr PCV pcv[BOTH] = {
                    encode_pcv(0, 0, 1, 1, 0, 0, 0, 0, 1, 0),
                    encode_pcv(0, 0, 0, 1, 0, 0, 0, 1, 1, 0)};
            };

            template <>
            struct SandboxPCV<ET_KQKP>
            {
                static constexpr PCV pcv[BOTH] = {
                    encode_pcv(0, 0, 0, 0, 1, 1, 0, 0, 0, 0),
                    encode_pcv(1, 0, 0, 0, 0, 0, 0, 0, 0, 1)};
            };

            template <>
            struct SandboxPCV<ET_KRKP>
            {
                static constexpr PCV pcv[BOTH] = {
                    encode_pcv(0, 0, 0, 1, 0, 1, 0, 0, 0, 0),
                    encode_pcv(1, 0, 0, 0, 0, 0, 0, 0, 1, 0)};
            };

            template <>
            struct SandboxPCV<ET_KNNK>
            {
                static constexpr PCV pcv[BOTH] = {
                    encode_pcv(0, 2, 0, 0, 0, 0, 0, 0, 0, 0),
                    encode_pcv(0, 0, 0, 0, 0, 0, 2, 0, 0, 0)};
            };

            template <>
            struct SandboxPCV<ET_KNNKP>
            {
                static constexpr PCV pcv[BOTH] = {
                    encode_pcv(0, 2, 0, 0, 0, 1, 0, 0, 0, 0),
                    encode_pcv(1, 0, 0, 0, 0, 0, 2, 0, 0, 0)};
            };

            template <>
            struct SandboxPCV<ET_KRKB>
            {
                static constexpr PCV pcv[BOTH] = {
                    encode_pcv(0, 0, 0, 1, 0, 0, 0, 1, 0, 0),
                    encode_pcv(0, 0, 1, 0, 0, 0, 0, 0, 1, 0)};
            };

            template <>
            struct SandboxPCV<ET_KRKN>
            {
                static constexpr PCV pcv[BOTH] = {
                    encode_pcv(0, 0, 0, 1, 0, 0, 1, 0, 0, 0),
                    encode_pcv(0, 1, 0, 0, 0, 0, 0, 0, 1, 0)};
            };

            template <EndgameType ET>
            class Endgame : public EndgameBase
            {
            public:
                Endgame(Color strong) : EndgameBase(strong) {}

                bool is_applicable(const Position &pos) const override
                {
                    return pos.get_pcv() == SandboxPCV<ET>::pcv[strong_side];
                }

                virtual ~Endgame() = default;

            protected:
                Value strong_side_score(const Position &pos) const override;
            };

            template <>
            Value Endgame<ET_KPK>::strong_side_score(const Position &pos) const
            {
                assert(is_applicable(pos));

                Color side = pos.side_to_move();
                Square strong_king_sq = pos.square<KING>(strong_side);
                Square weak_king_sq = pos.square<KING>(~strong_side);
                Square strong_pawn = pos.square<PAWN>(strong_side);

                BitBase::normalize(strong_side, side, strong_king_sq, strong_pawn, weak_king_sq);

                if (BitBase::check(side, strong_king_sq, strong_pawn, weak_king_sq))
                    return VALUE_POSITIVE_DRAW + Value(rank_of(sq_relative_to_side(strong_pawn, strong_side)));

                return VALUE_KNOWN_WIN + Value(rank_of(sq_relative_to_side(strong_pawn, strong_side)));
            }

            template <>
            bool Endgame<ET_KPsK>::is_applicable(const Position &pos) const
            {
                int nonpawns_count = pos.count(strong_side, KNIGHT, BISHOP, ROOK, QUEEN);
                int strong_pawns_count = pos.count(strong_side, PAWN);

                // Certain condition must be met in order for the King and pawns vs king [KPsK] endgame to be applicable
                return nonpawns_count == 0 &&                                              // No other pieces, except pawns
                       strong_pawns_count >= 2 &&                                          // Pawns must be more than one
                       pos.get_pieces_bb(weak_side) == pos.get_pieces_bb(KING, weak_side); // Get only the weak king
            }

            template <>
            Value Endgame<ET_KPsK>::strong_side_score(const Position &pos) const
            {
                const BITBOARD pawns = pos.get_pieces_bb(PAWN, strong_side);

                const File pawn_file = file_of(get_ls1b(pawns));

                const Square promotion_sq = sq_relative_to_side(make_square(pawn_file, RANK_8), strong_side);
                const Square weak_ksq = pos.square<KING>(weak_side);

                // It's theoretically a draw if:
                // 1. Pawns are on the A or H file
                // 2. Weak king can reach the promotion square

                if ((pawns == (pawns & FileA_Bits) || pawns == (pawns & FileH_Bits)) && distance(weak_ksq, promotion_sq) <= 1)
                    return VALUE_POSITIVE_DRAW;

                // Make a pawn strife for the promotion square
                return VALUE_KNOWN_WIN +
                       MATERIAL_SCORES.piece_value[PAWN].eg * pos.count<PAWN>(strong_side) +                    // Add strong pawn value
                       Value(rank_of(sq_relative_to_side(first_pawn_square(pawns, strong_side), strong_side))); // Add the rank of the first pawn
                                                                                                                // using first_pawn_square() instead of get_ls1b() because if let's say black is the strong side,
                                                                                                                // then least significant bit will be the backwards pawn
            }

            template <>
            Value Endgame<ET_KNBK>::strong_side_score(const Position &pos) const
            {
                Square weak_ksq = pos.square<KING>(weak_side);
                Square bishop = pos.square<BISHOP>(strong_side);
                Color bishop_color = square_color(bishop);

                // Flipping rankwise always results in the different color square
                Square ksq = (bishop_color == WHITE) ? flip_rankwise(weak_ksq) : weak_ksq;
                Value v = PUSH_TO_COLOR_CORNER_BONUS[ksq]; // Then this square is fed into the weights

                return Value(std::min(int64_t(VALUE_KNOWN_WIN + v), int64_t(VALUE_MATE - 1)));
            }

            template <>
            Value Endgame<ET_KQKR>::strong_side_score(const Position &pos) const
            {
                Square weak_ksq = pos.square<KING>(weak_side);
                Square strong_ksq = pos.square<KING>(strong_side);

                Value v = MATERIAL_SCORES.piece_value[QUEEN].eg -
                          MATERIAL_SCORES.piece_value[ROOK].eg +
                          PUSH_TO_EDGE_BONUS[weak_ksq] +
                          PUSH_CLOSER[distance<Rank>(strong_ksq, weak_ksq)]; // Because PUSH_CLOSER is with size RANK_NB = 8ULL

                return Value(std::min(int64_t(VALUE_KNOWN_WIN + v), int64_t(VALUE_MATE - 1)));
            }

            template <>
            bool Endgame<ET_KXK>::is_applicable(const Position &pos) const
            {
                // Using count_bits() and not count(Color, PieceTypes...) becase the weak side bitboard is mostly zeros
                return count_bits(pos.get_pieces_bb(weak_side)) == 1; // Only the weak king is present on the board
            }

            template <>
            Value Endgame<ET_KXK>::strong_side_score(const Position &pos) const
            {
                Square weak_ksq = pos.square<KING>(weak_side);
                Square strong_ksq = pos.square<KING>(strong_side);

                Value v = VALUE_DRAW;

                // Add strong piece value endgame score + their count
                for (const auto &pt : {PAWN, KNIGHT, BISHOP, ROOK, QUEEN})
                    v += MATERIAL_SCORES.piece_value[pt].eg * pos.count(strong_side, pt);

                v += PUSH_TO_EDGE_BONUS[weak_ksq] +
                     PUSH_CLOSER[distance<Rank>(strong_ksq, weak_ksq)]; // Because PUSH_CLOSER is with size RANK_NB = 8ULL

                return Value(std::min(int64_t(VALUE_KNOWN_WIN + v), int64_t(VALUE_MATE - 1)));
            }

            template <>
            Value Endgame<ET_KRNKR>::strong_side_score(const Position &pos) const
            {
                Square weak_ksq = pos.square<KING>(weak_side);
                return Value(VALUE_POSITIVE_DRAW + PUSH_TO_EDGE_BONUS[weak_ksq]);
            }

            template <>
            Value Endgame<ET_KRBKR>::strong_side_score(const Position &pos) const
            {
                Square weak_ksq = pos.square<KING>(weak_side);
                return Value(VALUE_POSITIVE_DRAW + PUSH_TO_EDGE_BONUS[weak_ksq]);
            }

            template <>
            bool Endgame<ET_KRBKR>::is_applicable(const Position &pos) const
            {
                return pos.get_pcv() == SandboxPCV<ET_KRBKR>::pcv[strong_side];
            }

            template <>
            bool Endgame<ET_KBPsK>::is_applicable(const Position &pos) const
            {
                return pos.count<BISHOP>(strong_side) == 1 &&                              // Only one string bishop
                       pos.count(strong_side, KNIGHT, ROOK, QUEEN) == 0 &&                 // No other pieces, except the bishop
                       pos.count<PAWN>(strong_side) > 0 &&                                 // At least one pawn
                       pos.get_pieces_bb(weak_side) == pos.get_pieces_bb(KING, weak_side); // Get only the weak king
            }

            template <>
            Value Endgame<ET_KBPsK>::strong_side_score(const Position &pos) const
            {
                const BITBOARD pawns = pos.get_pieces_bb(PAWN, strong_side);

                const File pawn_file = file_of(get_ls1b(pawns));
                const Square promotion_sq = sq_relative_to_side(make_square(pawn_file, RANK_8), strong_side);

                const Square weak_ksq = pos.square<KING>(weak_side);
                const Square bishop_sq = pos.square<BISHOP>(strong_side); // Only one bishop is present, so square<> will retrieve the only bishop

                // Theoretical draw if:
                // 1. All pawns are on the same file (A or H)
                // 2. Promotion color is different from bishop's color
                // 3. Weak king can reach the promotion square

                if ((pawns == (pawns & FileA_Bits) || pawns == (pawns & FileH_Bits)) &&                             // Rule 1
                    square_color(bishop_sq) != square_color(promotion_sq) && distance(weak_ksq, promotion_sq) <= 1) // Rule 2 and 3
                    return VALUE_POSITIVE_DRAW;

                // Make a pawn strife for the promotion square
                return VALUE_KNOWN_WIN +
                       MATERIAL_SCORES.piece_value[PAWN].eg * pos.count<PAWN>(strong_side) +                    // Add strong pawn value
                       MATERIAL_SCORES.piece_value[BISHOP].eg +                                                 // Add strong bishop value
                       Value(rank_of(sq_relative_to_side(first_pawn_square(pawns, strong_side), strong_side))); // Add the rank of the first pawn

                // using first_pawn_square() instead of get_ls1b() because if let's say black is the strong side,
                // then least significant bit will be the backwards pawn
            }

            template <>
            Value Endgame<ET_KRKP>::strong_side_score(const Position &pos) const
            {
                const Square weak_ksq = sq_relative_to_side(pos.square<KING>(weak_side), strong_side);
                const Square strong_ksq = sq_relative_to_side(pos.square<KING>(strong_side), strong_side);
                const Square pawn_sq = sq_relative_to_side(pos.square<PAWN>(strong_side), strong_side);
                const Square promotion_sq = make_square(file_of(pawn_sq), RANK_8); // Promotion square

                // if strong king is directly in front of the pawn
                if (strong_ksq < pawn_sq && distance<File>(strong_ksq, pawn_sq) <= 1)
                    return VALUE_KNOWN_WIN + PUSH_CLOSER[distance<Rank>(strong_ksq, weak_ksq)];

                // If pawn is advanced and supported by the king while the strong king is far away
                if (rank_of(pawn_sq) < RANK_5 && distance(weak_ksq, pawn_sq) <= 1 && distance(strong_ksq, pawn_sq) > 2)
                    return VALUE_POSITIVE_DRAW + Value(rank_of(pawn_sq));

                return (MATERIAL_SCORES.piece_value[ROOK] - MATERIAL_SCORES.piece_value[PAWN]).eg -
                       PUSH_CLOSER[distance<Rank>(pawn_sq, promotion_sq)];
            }

            template <>
            Value Endgame<ET_KNNK>::strong_side_score(const Position &) const { return VALUE_DRAW; }

            template <>
            Value Endgame<ET_KNNKP>::strong_side_score(const Position &pos) const
            {
                const Square weak_ksq = sq_relative_to_side(pos.square<KING>(weak_side), strong_side);
                const Square strong_ksq = sq_relative_to_side(pos.square<KING>(strong_side), strong_side);
                const Square pawn_sq = sq_relative_to_side(pos.square<PAWN>(strong_side), strong_side);

                BITBOARD knights = pos.get_pieces_bb(KNIGHT, strong_side);

                const Square k1 = sq_relative_to_side(pop_ls1b(knights), strong_side);
                const Square k2 = sq_relative_to_side(pop_ls1b(knights), strong_side);

                // Try to push the weak king to the corner and don't allow pawn to be pushed too far
                // Keep strong pieces (king and knights) close to the weak king

                return MATERIAL_SCORES.piece_value[PAWN].eg +
                       5 * (PUSH_CLOSER[distance<Rank>(strong_ksq, weak_ksq)] +
                            PUSH_CLOSER[distance<Rank>(k1, weak_ksq)] +
                            PUSH_CLOSER[distance<Rank>(k2, weak_ksq)] +
                            6 * Value(rank_of(pawn_sq))) +
                       PUSH_TO_EDGE_BONUS[weak_ksq];
            }

            template <>
            bool Endgame<ET_KBPsKB>::is_applicable(const Position &pos) const
            {
                return pos.count<BISHOP>(strong_side) == 1 &&                // Only one string bishop
                       pos.count<BISHOP>(weak_side) == 1 &&                  // Only one weak bishop
                       pos.count<PAWN>(strong_side) >= 1 &&                  // At least one pawn
                       pos.count(strong_side, KNIGHT, ROOK, QUEEN) == 0 &&   // No other strong pieces
                       pos.count(weak_side, PAWN, KNIGHT, ROOK, QUEEN) == 0; // No weak pieces or pawns
            }

            template <>
            Value Endgame<ET_KBPsKB>::strong_side_score(const Position &pos) const
            {
                const BITBOARD pawns = pos.get_pieces_bb(PAWN, strong_side);

                const Square furthest_pawn_sq = sq_relative_to_side(first_pawn_square(pawns, strong_side), strong_side);
                const Square weak_ksq = sq_relative_to_side(pos.square<KING>(weak_side), strong_side);
                const Square strong_bishop_sq = sq_relative_to_side(pos.square<BISHOP>(strong_side), strong_side);
                const Square weak_bishop_sq = sq_relative_to_side(pos.square<BISHOP>(weak_side), strong_side); // Only one bishop is present, so square<> will retrieve the only bishop

                // In draw cases try not to lose pawns and push them if possible
                bool on_same_file = pawns == (pawns & FileA_Bits) || pawns == (pawns & FileB_Bits) ||
                                    pawns == (pawns & FileC_Bits) || pawns == (pawns & FileD_Bits) ||
                                    pawns == (pawns & FileE_Bits) || pawns == (pawns & FileF_Bits) ||
                                    pawns == (pawns & FileG_Bits) || pawns == (pawns & FileH_Bits);

                if (on_same_file)
                {
                    if (file_of(weak_ksq) == file_of(furthest_pawn_sq) &&
                        rank_of(weak_ksq) > rank_of(furthest_pawn_sq) &&
                        square_color(weak_ksq) != square_color(strong_bishop_sq))
                        return VALUE_POSITIVE_DRAW + 2 * (5 * Value(pos.count<PAWN>(strong_side)) + Value(rank_of(furthest_pawn_sq)));
                }
                else if (square_color(strong_bishop_sq) != square_color(weak_ksq))
                {
                    bool pawn_on_file[FILE_NB] = {false};
                    File furthest_pawn_file = file_of(furthest_pawn_sq);
                    File f2 = furthest_pawn_file;

                    BITBOARD pp = pawns;

                    while (pp)
                    {
                        File current_pawn_file = file_of(pop_ls1b(pp));

                        if (current_pawn_file != furthest_pawn_file)
                            f2 = current_pawn_file;

                        pawn_on_file[current_pawn_file] = true;
                    }

                    // This sums up the number of files
                    // where bool * __first = pawn_on_file is actually &pawn_on_file[0]
                    // and bool * __last = pawn_on_file + FILE_NB is actually &pawn_on_file[FILE_NB]
                    // and 0 is the initial value of the sum
                    int file_count = std::accumulate(pawn_on_file, pawn_on_file + FILE_NB, 0);

                    if (file_count == 2)
                    {
                        const Square furthest_pawn_2_sq = sq_relative_to_side((first_pawn_square(pawns & file_bb(f2), strong_side)), strong_side);

                        if (edge_distance(furthest_pawn_file, f2) == 1 &&                     // if pawns are on neighbouring files
                            !has_bit_after_pop(pawns & file_bb(f2)) &&                        // AND there is no other pawn on the file
                            rank_of(furthest_pawn_sq) < rank_of(furthest_pawn_2_sq) &&        // AND the furthest pawn is on a higher rank than the other pawn
                            square_color(furthest_pawn_sq) == square_color(strong_bishop_sq)) // AND the furthest pawn is on the same color as the strong bishop
                        {
                            const Square block1_sq = make_square(furthest_pawn_file, rank_of(furthest_pawn_sq) + 1);
                            const Square block2_sq = make_square(f2, rank_of(furthest_pawn_sq));

                            if (weak_ksq == block1_sq &&
                                (weak_bishop_sq == block2_sq ||
                                 attacks_bb_by<BISHOP>(weak_bishop_sq, pos.get_all_pieces_bb() & block2_sq)))
                            {
                                return VALUE_POSITIVE_DRAW +
                                       2 * (5 * Value(pos.count<PAWN>(strong_side)) +
                                            Value(rank_of(furthest_pawn_sq)));
                            }
                            if (weak_ksq == block2_sq &&
                                (weak_bishop_sq == block1_sq ||
                                 attacks_bb_by<BISHOP>(weak_bishop_sq, pos.get_all_pieces_bb() & block1_sq)))
                            {
                                return VALUE_POSITIVE_DRAW +
                                       2 * (5 * Value(pos.count<PAWN>(strong_side)) +
                                            Value(rank_of(furthest_pawn_sq)));
                            }
                        }
                    }
                }

                return pos.count<PAWN>(strong_side) * MATERIAL_SCORES.piece_value[PAWN].eg + 10 * Value(rank_of(furthest_pawn_sq));
            }

        }; // namespace

    } // namespace Endgame
} // namespace KhaosChess