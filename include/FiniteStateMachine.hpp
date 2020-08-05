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
                return node;
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

    std::vector<std::unique_ptr<token::Token>> tokenizeText(const std::string &text)
    {
        std::vector<std::unique_ptr<token::Token>> tokens;
        auto processChar = [&](char c) {
            auto newToken = followChar(c);
            if (newToken)
            {
                tokens.emplace_back(std::move(newToken));
            }
        };
        std::for_each(text.begin(), text.end(), processChar);
        processChar('\0');
        return tokens;
    }

    std::shared_ptr<Node> getRoot() const
    {
        return root_;
    }

private:
    // Returns new Token pointer, if terminal node is reached, nullptr otherwise
    // Does NOT includes char, that followed to the terminal
    // TODO: is it correct?
    // Token resets on root
    std::unique_ptr<token::Token> followChar(char c)
    {
        auto nextNode_ = curNode_->getNextNode(c);
        std::unique_ptr<token::Token> result;
        if (nextNode_ && nextNode_->isTerminal())
        {
            auto tokenLen = curTokenString_.length();
            result = std::make_unique<token::Token>(nextNode_->getTokenCategory(),
                                             std::move(curTokenString_),
                                             curTokenPos_ - tokenLen);
            curNode_ = root_;
            curTokenString_.clear();
            followChar(c); // TODO: infinite recursion check
        }
        else if (!nextNode_)
        {
            // TODO: maybe this action may be customized ???
            throw std::invalid_argument("can not follow char on position " + std::to_string(curTokenPos_));
        }
        else
        {
            ++curTokenPos_;
            curTokenString_ += c;
            curNode_ = nextNode_;
            if (curNode_ == root_)
            {
                curTokenString_.clear();
            }
        }
        return result;
    }

    std::shared_ptr<Node> root_;
    std::shared_ptr<Node> curNode_;
    std::string curTokenString_ = "";
    int curTokenPos_ = 0;
};
} // namespace fsm
} // namespace peach
