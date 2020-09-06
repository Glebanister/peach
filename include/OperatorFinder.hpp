#pragma once

#include <array>
#include <stdexcept>

#include "FiniteStateMachine.hpp"

namespace peach
{
namespace fsm
{
// Finite state machine, finds tokens with category tokenCategory::OPERATOR
class OperatorFinder : public FiniteStateMachine
{
public:
    OperatorFinder(const std::vector<std::pair<std::string, token::tokenCategory_t>> &operators)
    {
        std::for_each(operators.begin(), operators.end(), [&](const auto &pat) {
            addOperatorPattern(pat.first, pat.second);
        });
    }

    // Adds pattern to finite state machine.
    // Should not contain latin letters, digits, separators or underscores.
    void addOperatorPattern(const std::string &pattern, token::tokenCategory_t category)
    {
        if (pattern.length() == 0)
        {
            throw std::invalid_argument("pattern must contain at least one character");
        }
        auto curNode = getRoot();
        std::array<std::unique_ptr<transition::CharTransition>, 2> badTransitions =
            {std::make_unique<transition::LatinUnderscoreDigitTransition>(),
             std::make_unique<transition::SetCharTransition>(std::vector<char>{' ', '\n', '\t'})}; // TODO: define what is separator
        for (char c : pattern)
        {
            for (auto &tr : badTransitions)
            {
                if (tr->isActive(c))
                {
                    throw std::invalid_argument("character " + std::to_string(c) + " is not allowed in operators");
                }
            }
            if (curNode->getNextNode(c))
            {
                curNode = curNode->getNextNode(c);
            }
            else
            {
                curNode = curNode->addTransitionToNewNode<transition::SingleCharTransition>(token::tokenCategory::UNDEFINED, c);
            }
        }
        curNode->addTransitionToNewNode<transition::TrueTransition>(category);
    }
};
} // namespace fsm
} // namespace peach
