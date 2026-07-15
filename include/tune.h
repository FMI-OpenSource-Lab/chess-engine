#pragma once

#include <iosfwd>
#include <string>
#include <vector>

#include "defs.h"

namespace KhaosChess {
struct TunableParam {
    std::string name;
    Value* slot;
};

// Registry of every live evaluation weight
const std::vector<TunableParam>& tunable_params();

// Set one weight by name; return false if the name is unknown
bool set_param(const std::string& name, Value v);

// Dump all weights
void print_params(std::ostream& os);

}  // namespace KhaosChess
