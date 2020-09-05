#pragma once

#include "FiniteStateMachine.hpp"

namespace peach
{
namespace fsm
{
// Finite state machine, finds number-like tokens
// decimalSeparator (usually dot or comma) - separates integer and fractional parts. Can't be digit.
// lastCharacter - character, that numbers of this type has, like d for double in C++ or f for float
class NumberFinder : public FiniteStateMachine
{
public:
    // For integer without lastCharacter
    NumberFinder(token::tokenCategory_t category)
    {
        auto firstNode = addMinusOrDigitTransitionToNewNode(getRoot());
        addDigitLoop(firstNode);
        firstNode->addTransitionToNewNode<transition::TransitionNegation<transition::DigitCharTransition>>(category);
    }

    // For floating point number without lastCharacter
    NumberFinder(token::tokenCategory_t category, char decimalSeparator)
    {
        if ('0' <= decimalSeparator && decimalSeparator <= '9')
        {
            throw std::invalid_argument("decimalSeparator can not be a digit");
        }
        auto firstNode = addMinusOrDigitTransitionToNewNode(getRoot());
        addDigitLoop(firstNode);
        auto secondNode = firstNode->addTransitionToNewNode<transition::SingleCharTransition>(token::tokenCategory::UNDEFINED, decimalSeparator);
        addDigitLoop(secondNode);
        secondNode->addTransitionToNewNode<transition::TransitionNegation<transition::DigitCharTransition>>(category);
    }

    // For integer with lastCharacter
    // Third parameter is not used, if you want to use this constructor, you should set third parameter to something
    NumberFinder(token::tokenCategory_t category, char lastCharacter, bool)
    {
        auto firstNode = addMinusOrDigitTransitionToNewNode(getRoot());
        addDigitLoop(firstNode);
        auto secondNode = firstNode->addTransitionToNewNode<transition::SingleCharTransition>(token::tokenCategory::UNDEFINED, lastCharacter);
        secondNode->addTransitionToNewNode<transition::TrueTransition>(category);
    }

    // For floating point number with last character
    NumberFinder(token::tokenCategory_t category, char decimalSeparator, char lastCharacter)
    {
        if ('0' <= decimalSeparator && decimalSeparator <= '9')
        {
            throw std::invalid_argument("decimalSeparator can not be a digit");
        }
        auto firstNode = addMinusOrDigitTransitionToNewNode(getRoot());
        addDigitLoop(firstNode);
        auto secondNode = firstNode->addTransitionToNewNode<transition::SingleCharTransition>(token::tokenCategory::UNDEFINED, decimalSeparator);
        addDigitLoop(secondNode);
        auto terminal = secondNode->addTransitionToNewNode<transition::SingleCharTransition>(token::tokenCategory::UNDEFINED, lastCharacter);
        terminal->addTransitionToNewNode<transition::TrueTransition>(category);
    }

private:
    // Creates loops it on itself with DigitCharTransition
    void addDigitLoop(std::shared_ptr<fsm::Node> nodeFrom)
    {
        if (!nodeFrom)
        {
            throw std::invalid_argument("nodeFrom is nullptr");
        }
        nodeFrom->addTransition<transition::DigitCharTransition>(nodeFrom);
    }

    // Creates node from nodeFrom with DigitCharTransition and loops it on itself with DigitCharTransition
    // Returns new node
    std::shared_ptr<fsm::Node> addDigitLoopOnNewNode(std::shared_ptr<fsm::Node> nodeFrom)
    {
        if (!nodeFrom)
        {
            throw std::invalid_argument("nodeFrom is nullptr");
        }
        auto newNode = nodeFrom->addTransitionToNewNode<transition::DigitCharTransition>(token::tokenCategory::UNDEFINED);
        addDigitLoop(newNode);
        return newNode;
    }

    // Adds transition from given node to new by '-' or digit
    // Returns new node
    std::shared_ptr<fsm::Node> addMinusOrDigitTransitionToNewNode(std::shared_ptr<fsm::Node> nodeFrom)
    {
        if (!nodeFrom)
        {
            throw std::invalid_argument("nodeFrom is nullptr");
        }
        auto node1 = nodeFrom->addTransitionToNewNode<transition::SingleCharTransitionTemplate<'-'>>(token::tokenCategory::UNDEFINED);
        auto term = node1->addTransitionToNewNode<transition::DigitCharTransition>(token::tokenCategory::UNDEFINED);
        nodeFrom->addTransition<transition::DigitCharTransition>(term);
        return term;
    }
};
} // namespace fsm
} // namespace peach
