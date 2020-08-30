#pragma once

#include <ostream>
#include <string>
#include <utility>

namespace peach
{
namespace token
{
enum class tokenCategory
{
    /*
    Token is undefined
    */
    UNDEFINED,

    /*
    Token is a name,
    Starts with a latin letter,
    May contain:
        - contains digits
        - latin letters
        - underscores
    Examples:
        - 'VariableName1'
        - 'variable_name_2' 
    */
    NAME,

    /*
    Token is a value - predefined, or user-defined,
    Examples:
        - '123'                for std::int32_t
        - '"this is a string"' for std::string
    */
    VALUE,

    /*
    Token is an binary operator,
    Descending priority:
        - '&'        logical and
        - '|'        logical or
        - '*'        multiplication
        - '/'        division
        - '%'        module
        - '+'        addition
        - '-'        substraction
    */
    OPERATOR_BI,

    /*
    Token is an unary operator,
    Descending priority:
        - '!'        logical negation
    */
    OPERATOR_UN,

    /*
    Token states for assignment,
    Only name token may be on the left,
    Any expression could be on the right.
        - '='  assign
        - 'X=' X and assign
    */
    ASSIGNMENT,

    /*
    Token is 'if' in conditional chain
    */
    COND_IF,

    /*
    Token is 'elif' in conditional chain
    */
    COND_ELIF,

    /*
    Token is 'else' in conditional chain
    */
    COND_ELSE,

    // TODO
    // /*
    // Token is 'for' loop
    // */
    // LOOP_FOR,

    /*
    Token is 'while' loop
    */
    LOOP_WHILE,

    /*
    Token is a opening round bracket
    */
    BRACKET_OPEN,

    /*
    Token is a closing round bracket
    */
    BRACKET_CLOSE,

    /*
    Token is an endline character
    */
    SEP_ENDL,

    /*
    Token is a tabulation character
    */
    SEP_TAB,

    /*
    Token is a space character
    */
    SEP_SPACE,

    // TODO
    // /*
    // Token is a begin of function declaration 'func'
    // */
    // FUNC_DECL,

    // TODO
    // /*
    // Token is return in function 'return'
    // */
    // FUNC_RETURN,

    _TOKEN_TOTAL,
};

std::ostream &operator<<(std::ostream &os, tokenCategory category)
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
    };
    return os << tokenCategoryString[static_cast<std::size_t>(category)];
}

class Token
{
public:
    Token(tokenCategory category,
          std::string token,
          int position)
        : category_(std::move(category)),
          token_(std::move(token)),
          position_(std::move(position))
    {
    }

    tokenCategory getCategory() const noexcept
    {
        return category_;
    }

    std::string getTokenString() const
    {
        return token_;
    }

    int getPosition() const noexcept
    {
        return position_;
    }

    void setCategory(tokenCategory category) noexcept
    {
        category_ = category;
    }

private:
    tokenCategory category_;
    std::string token_;
    int position_;
};
} // namespace token
} // namespace peach
