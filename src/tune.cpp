#include "tune.h"

#include <ostream>

#include "score.h"

namespace KhaosChess {
namespace {
const char* PIECE_NAMES[PIECE_TYPE_NB] = {"none", "pawn", "knight", "bishop",
                                          "rook", "queen", "king"};

void add(std::vector<TunableParam>& reg, const std::string& name, Value& v) {
    reg.push_back({name, &v});
}

void add(std::vector<TunableParam>& reg, const std::string& name, Score& s) {
    add(reg, name + ".mg", s.mg);
    add(reg, name + ".eg", s.eg);
}

std::vector<TunableParam> build_registry() {
    std::vector<TunableParam> reg;

    for (PieceType pt = PAWN; pt <= QUEEN; ++pt)
        add(reg, std::string("material.") + PIECE_NAMES[pt],
            MATERIAL_SCORES.piece_value[pt]);

    // mobility[PAWN], control_space[PAWN] and pinned_penalty are never
    // read by the eval, so they are not registered.
    for (PieceType pt = KNIGHT; pt <= KING; ++pt)
        add(reg, std::string("mobility.") + PIECE_NAMES[pt],
            PIECE_SCORES.mobility[pt]);

    for (PieceType pt = KNIGHT; pt <= QUEEN; ++pt)
        add(reg, std::string("control_space.") + PIECE_NAMES[pt],
            PIECE_SCORES.control_space[pt]);

    add(reg, "piece.bishop_pair", PIECE_SCORES.bishop_pair);
    add(reg, "piece.connected_rooks", PIECE_SCORES.connected_rooks);
    add(reg, "piece.outpost_knight", PIECE_SCORES.outpost_knight);
    add(reg, "piece.outpost_bishop", PIECE_SCORES.outpost_bishop);
    add(reg, "piece.safe_knight", PIECE_SCORES.safe_knight);
    add(reg, "piece.control_center_knight", PIECE_SCORES.control_center_knight);
    add(reg, "piece.vulnerable_queen", PIECE_SCORES.vulnerable_queen);
    add(reg, "piece.rook_open_file", PIECE_SCORES.rook_open_file);
    add(reg, "piece.rook_semi_open_file", PIECE_SCORES.rook_semi_open_file);
    add(reg, "piece.trapped_rook", PIECE_SCORES.trapped_rook);

    for (int r = 1; r <= 6; ++r) {
        add(reg, "pawn.passed_rank_weight." + std::to_string(r),
            PAWN_STRUCTURE_SCORES.passed_rank_weight[r]);
        add(reg, "pawn.connected_bonus." + std::to_string(r),
            PAWN_STRUCTURE_SCORES.connected_bonus[r]);
    }

    add(reg, "pawn.control_center", PAWN_STRUCTURE_SCORES.control_center);
    add(reg, "pawn.passed", PAWN_STRUCTURE_SCORES.passed);
    add(reg, "pawn.doubled", PAWN_STRUCTURE_SCORES.doubled);
    add(reg, "pawn.backward", PAWN_STRUCTURE_SCORES.backward);
    add(reg, "pawn.isolated", PAWN_STRUCTURE_SCORES.isolated);
    add(reg, "pawn.pawn_island", PAWN_STRUCTURE_SCORES.pawn_island);
    add(reg, "pawn.same_color_as_bishop",
        PAWN_STRUCTURE_SCORES.same_color_as_bishop);

    add(reg, "king.bonus", KING_SAFETY_SCORES.bonus);
    add(reg, "king.weak_diagonals", KING_SAFETY_SCORES.weak_diagonals);
    add(reg, "king.weak_lines", KING_SAFETY_SCORES.weak_lines);
    add(reg, "king.weak_back_rank", KING_SAFETY_SCORES.weak_back_rank);
    add(reg, "king.pawn_proximity_penalty",
        KING_SAFETY_SCORES.pawn_proximity_penalty);

    for (PieceType pt = KNIGHT; pt <= BISHOP; ++pt) {
        add(reg, std::string("king.protector_penalty.") + PIECE_NAMES[pt],
            KING_SAFETY_SCORES.protector_penalty[pt]);
        add(reg, std::string("king.attacker_penalty.") + PIECE_NAMES[pt],
            KING_SAFETY_SCORES.attacker_penalty[pt]);
    }

    add(reg, "tempo", TEMPO);

    for (PieceType pt = PAWN; pt <= KING; ++pt)
        for (Square s = A8; s <= H1; ++s) {
            if (pt == PAWN && (s <= H8 || s >= A1))
                continue;  // pawns never stand on the back ranks
            add(reg,
                std::string("psqt.") + PIECE_NAMES[pt] + "." +
                    std::to_string(int(s)),
                PSQT[pt][s]);
        }

    return reg;
}
}  // namespace

const std::vector<TunableParam>& tunable_params() {
    static std::vector<TunableParam> registry = build_registry();
    return registry;
}

bool set_param(const std::string& name, Value v) {
    for (const TunableParam& p : tunable_params()) {
        if (p.name == name) {
            *p.slot = v;
            return true;
        }
    }
    return false;
}

void print_params(std::ostream& os) {
    for (const TunableParam& p : tunable_params()) {
        os << p.name << " " << *p.slot << "\n";
    }
}
}  // namespace KhaosChess
