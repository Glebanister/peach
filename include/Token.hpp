#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <string>

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
    Token is an operator - unary or binary,
    Descending priority:
        - 'not', '!' logical negation
        - 'and', '&' logical and
        - 'or', '|'  logical or
        - '*'        multiplication
        - '/'        division
        - '%'        module
        - '+'        addition
        - '-'        substraction
    */
    OPERATOR,

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
};

class Token
{
public:
    Token(tokenCategory category)
        : category_(std::move(category)) {}

    tokenCategory getCategory() const noexcept
    {
        return category_;
    }

    virtual std::string_view getToken() const = 0;

private:
    tokenCategory category_;
};

template <tokenCategory Category>
class StringOwnerToken : public Token
{
public:
    StringOwnerToken(const std::string &token)
        : Token(Category),
          token_(token)
    {
    }

    StringOwnerToken(std::string &&token)
        : Token(Category),
          token_(std::move(token))
    {
    }

    std::string_view getToken() const override
    {
        return token_;
    }

private:
    std::string token_;
};

using UndefinedToken = StringOwnerToken<tokenCategory::UNDEFINED>;
using NameToken = StringOwnerToken<tokenCategory::NAME>;
} // namespace token
} // namespace peach

#endif // TOKEN_HPP
