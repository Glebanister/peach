#pragma once

#include <algorithm>
#include <cassert>
#include <ostream>
#include <stdexcept>
#include <string>
#include <string_view>

#include "Token.hpp"

namespace peach
{
namespace exception
{
struct PeachError
{
    PeachError(const std::string &name_,
               const std::string &description_)
        : name(name_), description(description_) {}

    const std::string name, description;
};

struct PositionalError : PeachError
{
    PositionalError(const std::string &name_,
                    const std::string &description_,
                    std::size_t textPosition_)
        : PeachError(name_, description_), position(textPosition_) {}
    const int position;
};

struct PositionalErrorPinter
{
    const PositionalError error;
    const std::string::iterator begin; // text begin
    const std::string::iterator end;   // text end
};

std::ostream &operator<<(std::ostream &os, const PeachError &error)
{
    return os << error.name << ": " << error.description;
}

std::ostream &operator<<(std::ostream &os, const PositionalErrorPinter &printer)
{
    // Returns char from program text by index
    auto text = [&](int index) {
        assert(index < std::distance(printer.begin, printer.end));
        return *(printer.begin + index);
    };
    int badStringEnd = std::distance(printer.begin, std::find(printer.begin, printer.end, '\n'));
    // if (badStringEnd == -1)
    // {
    //     badStringEnd = printer.text.size(); // TODO: one of corner cases
    // }
    int badStringBegin = badStringEnd - 1;
    for (; badStringBegin >= 1 && text(badStringBegin - 1) != '\n'; --badStringBegin)
        ;
    int badStringLineN = std::count(printer.begin + badStringBegin, printer.begin + badStringEnd, '\n');
    int badCharPosition = printer.error.position - badStringBegin;

    for (int i = badStringBegin; i < badStringEnd; ++i)
    {
        os << text(i);
    }
    os << '\n';

    for (int i = 0; i < badStringEnd - badStringBegin; ++i)
    {
        os << (badCharPosition == i ? "^" : "_");
    }
    os << PeachError(printer.error);
    os << "at position" << ' '
       << badStringLineN + 1 << ':'
       << badCharPosition + 1 << std::endl;
    return os;
}

namespace throwError
{
inline void indentation(int p) { throw PositionalError{"IndentationError", "invalid indentation", p}; }
inline void syntax(int p) { throw PositionalError{"SyntaxError", "invalid syntax", p}; }
inline void variableDeclaration(int p) { throw PositionalError{"VariableDeclarationError", "name expected", p}; }
inline void undefinedToken(int p) { throw PositionalError{"UndefinedTokenError", "token can not be recognized", p}; }
inline void unexpextedToken(int p) { throw PositionalError{"UnexpectedTokenError", "token is not expected", p}; }
inline void unexpectedElse(int p) { throw PositionalError{"UnexpectedElseError", "can not process 'else' if it not preceded by 'if'", p}; }
inline void undefinedOperator(int p) { throw PositionalError{"UndefinedOperatorError", "invalid syntax", p}; }
inline void bracketDisbalance(int p) { throw PositionalError{"BracketDisbalanceError", "can't match bracket", p}; }
inline void invalidAssignation(int p) { throw PositionalError{"InvalidAssignationError", "left expression must be variable access", p}; }
inline void unknownVariable(int p) { throw PositionalError{"UnknownVariableError", "variable is not visible", p}; }
inline void variableRedeclaration(int p) { throw PositionalError{"VariableRedeclarationError", "variable is declared already", p}; }
inline void interruption(int p) { throw PositionalError{"InterruptionError", "interpretation unexpectedly finished", p}; }
inline void zeroDivizion(int p) { throw PositionalError{"ZeroDivisionError", "can't divide by zero", p}; }
} // namespace throwError
} // namespace exception
} // namespace peach
