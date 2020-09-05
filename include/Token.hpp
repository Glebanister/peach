#pragma once

#include <ostream>
#include <string>
#include <utility>

namespace peach
{
namespace token
{
using tokenCategory_t = std::size_t;

struct tokenCategory
{
    tokenCategory() = delete;

    static constexpr tokenCategory_t
        UNDEFINED = 0,

        // Token is a name,
        // Starts with a latin letter,
        // May contain:
        //     - contains digits
        //     - latin letters
        //     - underscores
        // Examples:
        //     - 'VariableName1'
        //     - 'variable_name_2'
        NAME = 1,

        // Token is a value - predefined, or user-defined,
        // Examples:
        //     - '123'                for std::int32_t
        //     - '"this is a string"' for std::string
        VALUE = 2,

        // Token is an binary operator,
        // Descending priority:
        //     - '&'        logical and
        //     - '|'        logical or
        //     - '*'        multiplication
        //     - '/'        division
        //     - '%'        module
        //     - '+'        addition
        //     - '-'        substraction
        OPERATOR_BI = 3,

        // Token is an unary operator,
        // Descending priority:
        //     - '!'        logical negation
        OPERATOR_UN = 4,

        // Token states for assignment,
        // Only name token may be on the left,
        // Any expression could be on the right.
        //     - '='  assign
        //     - 'X=' X and assign
        ASSIGNMENT = 5,

        // Token is 'if' in conditional chain
        COND_IF = 6,

        // Token is 'elif' in conditional chain
        COND_ELIF = 7,

        // Token is 'else' in conditional chain
        COND_ELSE = 8,

        // Token is 'while' loop
        LOOP_WHILE = 9,

        // Token is a opening round bracket
        BRACKET_OPEN = 10,

        // Token is a closing round bracket
        BRACKET_CLOSE = 11,

        // Token is an endline character
        SEP_ENDL = 12,

        // Token is a tabulation character
        SEP_TAB = 13,

        // Token is a space character
        SEP_SPACE = 14,

        // Token is floating point number
        // Example: 420.69
        VALUE_FLOATING = 15,

        // Token is integer
        // Example: 239
        VALUE_INT = 16,

        // Token is colon ':'
        COLON = 17,

        // Token is semicolon ';'
        SEMICOLON = 18,

        _TOKEN_TOTAL = 19;
};

inline void printCategory(tokenCategory_t category)
{
    const std::array<std::string, static_cast<std::size_t>(tokenCategory::_TOKEN_TOTAL)> tokenCategoryString = {
        "UNDEFINED",
        "NAME",
        "VALUE",
        "OPERATOR_BI",
        "OPERATOR_UN",
        "ASSIGNMENT",
        "COND_IF",
        "COND_ELIF",
        "COND_ELSE",
        "LOOP_WHILE",
        "BRACKET_OPEN",
        "BRACKET_CLOSE",
        "SEP_ENDL",
        "SEP_TAB",
        "SEP_SPACE",
        "VALUE_FLOATING",
        "VALUE_INT",
        "COLON",
        "SEMICOLON",
    };
    std::cout << tokenCategoryString[static_cast<std::size_t>(category)];
}

class Token
{
public:
    Token(tokenCategory_t category,
          std::string token,
          int position)
        : category_(std::move(category)),
          token_(std::move(token)),
          position_(std::move(position))
    {
    }

    tokenCategory_t getCategory() const noexcept { return category_; }
    std::string getTokenString() const { return token_; }
    std::size_t getLine() const noexcept { return line_; } // TODO: calculate line!!!!
    int getPosition() const noexcept { return position_; }

    void setCategory(tokenCategory_t category) noexcept { category_ = category; }

private:
    tokenCategory_t category_;
    std::string token_;
    std::size_t line_, position_;
};

using TokenPtr = std::unique_ptr<Token>;

inline constexpr bool isEndline(tokenCategory_t category) noexcept
{
    return category == tokenCategory::SEP_ENDL;
}

inline bool isEndline(const TokenPtr &tk) noexcept
{
    return isEndline(tk->getCategory());
}

inline constexpr bool isSeparator(tokenCategory_t category) noexcept
{
    return category == tokenCategory::SEP_ENDL ||
           category == tokenCategory::SEP_SPACE ||
           category == tokenCategory::SEP_TAB;
}

inline bool isSeparator(const TokenPtr &tk) noexcept
{
    return isSeparator(tk->getCategory());
}

} // namespace token
} // namespace peach
