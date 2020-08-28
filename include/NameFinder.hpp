#pragma once

#include "FiniteStateMachine.hpp"

namespace peach
{
namespace fsm
{
// Finite state machine, finds tokens with category tokenCategory::NAME
class NameFinder : public FiniteStateMachine
{
public:
    NameFinder()
    {
        auto firstNode = getRoot()->addTransitionToNewNode<transition::LatinUnderscoreTransition>(token::tokenCategory::UNDEFINED);
        firstNode->addTransition<transition::LatinUnderscoreDigitTransition>(firstNode);
        firstNode->addTransitionToNewNode<transition::TransitionNegation<transition::LatinUnderscoreDigitTransition>>(token::tokenCategory::NAME);
    }
};
} // namespace fsm
} // namespace peach
