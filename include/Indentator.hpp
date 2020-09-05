#pragma once

#include <tuple>
#include <vector>

#include "Exception.hpp"
#include "Token.hpp"

namespace peach
{
namespace interpreter
{
using TokenVector = std::vector<token::TokenPtr>;
using TokenIterator = TokenVector::iterator;

class Indentator
{
public:
    // Separates indentation from tokens
    // Accepts first token of line, last code token and single indentation block structure
    // Returns pair of [number of indentation blocks; first token after indentation iterator]
    // Ignores blank lines!
    // Indentation of each line must be able to be defined without context of other lines
    // Throws peach::exception::IndentationException in bad case
    static std::pair<std::size_t, TokenIterator> getIndentation(TokenIterator tokensBegin, // first line token
                                                                TokenIterator tokensEnd,   // last code token
                                                                const std::vector<token::tokenCategory_t> &singleIndentationBlock)
    {
        auto curBlockPos = singleIndentationBlock.begin();
        std::size_t indentationBlocksCount = 0;
        auto curToken = tokensBegin;
        for (;; ++curToken, ++curBlockPos)
        {
            if (curBlockPos == singleIndentationBlock.end())
            {
                ++indentationBlocksCount;
                curBlockPos = singleIndentationBlock.begin();
            }
            if (curToken == tokensEnd)
            {
                return {0, tokensEnd};
            }

            if (token::isEndline(*curToken))
            {
                indentationBlocksCount = 0;
                curBlockPos = singleIndentationBlock.begin() - 1; // TODO: this is not nice
            }
            else if ((*curToken)->getCategory() != *curBlockPos)
            {
                if (curBlockPos != singleIndentationBlock.begin())
                {
                    exception::throwFromTokenIterator<exception::IndentationError>(curToken); // TODO: not as fast as user may want
                                                                                              // Maybe we can return std::variant<..., Exception>
                }
                else
                {
                    return {indentationBlocksCount, curToken};
                }
            }
        }
    }
};
} // namespace interpreter
} // namespace peach
