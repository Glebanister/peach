#pragma once

#include "FiniteStateMachine.hpp"

namespace peach
{
namespace fsm
{
// Name here - pattern that starts with latin letter or underscore, contains latin letters, underscores and digits
class NameFinder : protected FiniteStateMachine
{
public:
    NameFinder()
    {
        auto firstNode = getRoot()->addTransitionToNewNode<transition::LatinUnderscoreTransition>(token::tokenCategory::UNDEFINED);
        firstNode->addTransition<transition::LatinUnderscoreDigitTransition>(firstNode);
        firstNode->addTransitionToNewNode<transition::TransitionNegation<transition::LatinUnderscoreDigitTransition>>(token::tokenCategory::NAME);
        getRoot()->addTransition<transition::TransitionNegation<transition::LatinUnderscoreTransition>>(getRoot());
    }

    std::vector<std::unique_ptr<token::Token>> tokenizeText(const std::string &text,
                                                     const std::unordered_map<std::string, token::tokenCategory> &reservedNames = {})
    {
        auto tokens = FiniteStateMachine::tokenizeText(text);
        static_assert(static_cast<int>(token::tokenCategory::UNDEFINED) == 0); // for unordered map
        for (auto &token : tokens)
        {
            auto string = token->getTokenString();
            auto it = reservedNames.find(string);
            if (it == reservedNames.cend())
            {
                continue;
            }
            token->setCategory(it->second);
        }
        return tokens;
    }
};
} // namespace fsm
} // namespace peach
