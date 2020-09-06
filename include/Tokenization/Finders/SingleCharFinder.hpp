#pragma once

#include <array>
#include <cassert>

#include "FiniteStateMachine.hpp"

namespace peach
{
namespace fsm
{
// Finite state machine, finds tokens which contain single character.
class SingleCharFinder : public FiniteStateMachine
{
public:
    SingleCharFinder(const std::vector<std::pair<char, token::tokenCategory_t>> &chars)
    {
        for (const auto &[ch, category] : chars)
        {
            auto charNode = getRoot()->addTransitionToNewNode<transition::SingleCharTransition>(token::tokenCategory::UNDEFINED, ch);
            charNode->addTransitionToNewNode<transition::TrueTransition>(category);
        }
    }
};
} // namespace fsm
} // namespace peach
