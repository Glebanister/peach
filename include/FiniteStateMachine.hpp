#pragma once

#include <memory>

#include "Token.hpp"
#include "Transition.hpp"

namespace peach
{
namespace fsm
{
class Node
{
public:
    Node(token::tokenCategory category = token::tokenCategory::UNDEFINED)
        : category_(category)
    {
    }

    template <typename Transition, typename... TransitionArgs>
    void addTransition(std::shared_ptr<Node> nextNode, TransitionArgs &&... args)
    {
        auto newTransition = std::make_unique<Transition>(std::forward<TransitionArgs>(args)...);
        transitions_.emplace_back(std::move(newTransition), std::move(nextNode));
    }

    template <typename Transition, typename... TransitionArgs>
    std::shared_ptr<Node> addTransitionToNewNode(token::tokenCategory category, TransitionArgs &&... args)
    {
        auto newNode = std::make_shared<Node>(category);
        addTransition<Transition>(newNode, std::forward<TransitionArgs>(args)...);
        return newNode;
    }

    std::shared_ptr<Node> getNextNode(char c)
    {
        for (auto &&[transition, node] : transitions_)
        {
            if (transition->isActive(c))
            {
                return std::shared_ptr<Node>(node);
            }
        }
        return nullptr;
    }

    bool isTerminal() const noexcept
    {
        return category_ != token::tokenCategory::UNDEFINED;
    }

    token::tokenCategory getTokenCategory() const noexcept
    {
        return category_;
    }

private:
    std::vector<std::pair<std::unique_ptr<transition::CharTransition>, std::shared_ptr<Node>>> transitions_;
    token::tokenCategory category_;
};

class FiniteStateMachine
{
public:
    FiniteStateMachine()
        : root_(std::make_shared<Node>()),
          curNode_(root_)
    {
    }

    std::shared_ptr<Node> getRoot() const noexcept
    {
        return root_;
    }

    std::shared_ptr<Node> getCurrentNode() const noexcept
    {
        return curNode_;
    }

    // Follows char according to inner graph. If transition can not be processed, current node assigns to root
    // Returns pair of [was push successfull; category of current node].
    // Current node resets to root if terminal has been reached.
    std::pair<bool, token::tokenCategory> pushChar(char c)
    {
        auto nextNode = curNode_->getNextNode(c);
        token::tokenCategory prevNodeCategory = curNode_->getTokenCategory();
        bool success = true;
        if (!nextNode)
        {
            curNode_ = root_;
            success = false;
        }
        else
        {
            curNode_ = nextNode;
            if (curNode_->isTerminal())
            {
                prevNodeCategory = curNode_->getTokenCategory();
                curNode_ = root_;
            }
        }
        return {success, prevNodeCategory};
    }

private:
    std::shared_ptr<Node> root_;
    std::shared_ptr<Node> curNode_;
};
} // namespace fsm
} // namespace peach
