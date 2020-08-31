#pragma once

#include <string>
#include <unordered_map>

namespace peach
{
namespace expression
{
// Variables type
using VType = std::int32_t;

// Scope is the current state of visible variables
using Scope = std::unordered_map<std::string, VType>; // TODO: wtf is this

struct RvalueAndScope
{
    VType rvalue;
    Scope scope;
};

class Expression
{
public:
    virtual RvalueAndScope eval() = 0;
};
} // namespace expression
} // namespace peach
