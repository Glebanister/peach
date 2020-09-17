#pragma once

#include <array>
#include <cassert>

#include "FiniteStateMachine.hpp"

namespace peach
{
namespace fsm
{
// Finite state machine, finds string literals.
// escaped characters: [escaped, real]
template <char Separator>
class LiteralFinder : public FiniteStateMachine
{
public:
    LiteralFinder(token::tokenCategory_t category)
    {
        auto node = getRoot()->template addTransitionToNewNode<transition::SingleCharTransitionTemplate<Separator>>(token::tokenCategory::UNDEFINED);
        node->template addTransitionToNewNode<transition::SingleCharTransitionTemplate<'\n'>>(token::tokenCategory::UNDEFINED);
        node->template addTransition<transition::TransitionNegation<transition::SingleCharTransitionTemplate<Separator>>>(node);
        auto terminal = node->template addTransitionToNewNode<transition::SingleCharTransitionTemplate<Separator>>(token::tokenCategory::UNDEFINED);
        terminal->template addTransitionToNewNode<transition::TrueTransition>(category);
    }
};
} // namespace fsm
} // namespace peach
