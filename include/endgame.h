#pragma once

#include "position.h"
#include "score.h"

#include <array>
#include <functional>

namespace KhaosChess
{
    namespace BitBase
    {
        void init();
        bool check(Color side, Square w_ksq, Square w_pawn, Square b_ksq);
        void normalize(Color strong_side, Color &side, Square &strong_king, Square &string_pawn, Square &weak_king);
    } // namespace BitBase

    namespace Endgames
    {
        class EndgameBase
        {
        public:
            EndgameBase(Color strong)
                : strong_side(strong),
                  weak_side(~strong) {}

            // Verifies if the endgame is applicable to the given position
            virtual bool is_applicable(const Position &pos) const = 0;

            // Scores given position, assuming it is applicable
            Value score (const Position& pos) const
            {
                Value v = strong_side_score(pos);
                return pos.side_to_move() == strong_side ? v : -v;
            }

            virtual ~EndgameBase() = default;

        protected:
            // Scores given position from the strong side's perspective, assuming it is applicable
            virtual Value strong_side_score(const Position &pos) const = 0;

            Color strong_side, weak_side;
        };

        void init();
        Value score(const Position &pos);

    } // namespace Endgames

} // namespace KhaosChess