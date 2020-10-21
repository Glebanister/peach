#pragma once

#include <iostream>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

namespace peach
{
namespace token
{
enum class tokenCategory
{
    UNDEFINED,
    IDENTIFIER,
    INTEGER,
    OPERATOR,
    BRACKET_OPEN,
    BRACKET_CLOSE,
    SEP,
    COLON,
    SEMICOLON,
    _TOKEN_TOTAL
};

struct Token
{
    tokenCategory category;
    std::string token;
    std::size_t textPosition;
};

using TokenPtr = std::unique_ptr<Token>;

inline bool isEndline(const Token &token) noexcept
{
    return token.token == "\n";
}

inline bool isEndline(const TokenPtr &tk) noexcept
{
    return isEndline(*tk);
}

inline bool isSeparator(const Token &token) noexcept
{
    return token.category == tokenCategory::SEP;
}

inline bool isSeparator(const TokenPtr &tk) noexcept
{
    return isSeparator(*tk);
}

std::ostream &operator<<(std::ostream &os, tokenCategory category)
{
    std::string stringView;
    switch (category)
    {
    case tokenCategory::UNDEFINED:
        stringView = "undefined";
        break;
    case tokenCategory::IDENTIFIER:
        stringView = "identifier";
        break;
    case tokenCategory::INTEGER:
        stringView = "integer";
        break;
    case tokenCategory::OPERATOR:
        stringView = "operator";
        break;
    case tokenCategory::BRACKET_OPEN:
        stringView = "opening bracket";
        break;
    case tokenCategory::BRACKET_CLOSE:
        stringView = "closing bracket";
        break;
    case tokenCategory::SEP:
        stringView = "separator";
        break;
    case tokenCategory::COLON:
        stringView = "colon";
        break;
    case tokenCategory::SEMICOLON:
        stringView = "semicolon";
        break;
    default:
        stringView = "unrecognized token category";
        break;
    }
    os << "[" << stringView << "]";
}

std::ostream &operator<<(std::ostream &os, const Token &token)
{
    return os << token.category << " '" << token.token << "' at " << token.textPosition;
}
} // namespace token
} // namespace peach
