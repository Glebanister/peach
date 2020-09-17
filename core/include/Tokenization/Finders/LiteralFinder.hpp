#pragma once

#include <array>
#include <cassert>

#include "FiniteStateMachine.hpp"

namespace peach
{
namespace fsm
{
// Finite state machine, finds string literals.

template <char Separator>
class LiteralFinder : public FiniteStateMachine
{
public:
    LiteralFinder(token::tokenCategory_t category)
    {
        auto node = getRoot()->template addTransitionToNewNode<transition::SingleCharTransitionTemplate<Separator>>(token::tokenCategory::UNDEFINED);
        node->template addTransition<transition::TransitionNegation<transition::SingleCharTransitionTemplate<Separator>>>(node);
        node->template addTransitionToNewNode<transition::SingleCharTransitionTemplate<Separator>>(category);
    }
};
} // namespace fsm
} // namespace peach
