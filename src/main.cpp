#include <algorithm>
#include <iostream>
#include <memory>
#include <numeric>
#include <unordered_map>
#include <vector>

#include "Token.hpp"

using namespace peach;

class CharTransition
{
public:
    virtual bool isActive(char) = 0;
    virtual ~CharTransition() = default;
};

class RangeCharTransition : public CharTransition
{
public:
    RangeCharTransition(char begin, char end)
        : begin_(begin),
          end_(end)
    {
    }

    bool isActive(char c) override
    {
        return begin_ <= c && c <= end_;
    }

private:
    char begin_;
    char end_;
};

class SingleCharTransition : public RangeCharTransition
{
public:
    SingleCharTransition(char c)
        : RangeCharTransition(c, c) {}
};

class SetCharTransition : public CharTransition
{
public:
    SetCharTransition(std::initializer_list<char> charset)
        : charset_(std::move(charset)) {}

    bool isActive(char c) override
    {
        return std::find(charset_.begin(), charset_.end(), c) < charset_.end();
    }

private:
    std::vector<char> charset_;
};

template <char Begin, char End>
class RangeCharTransitionTemplate : public RangeCharTransition
{
public:
    RangeCharTransitionTemplate()
        : RangeCharTransition(Begin, End) {}
};

using DigitCharTransition = RangeCharTransitionTemplate<'0', '9'>;
using LowerLatinCharTransition = RangeCharTransitionTemplate<'a', 'z'>;
using UpperLatinCharTransition = RangeCharTransitionTemplate<'A', 'Z'>;

class LatinUnderscoreTransition : public CharTransition
{
public:
    bool isActive(char c) override
    {
        return (('a' <= c && c <= 'z') ||
                ('A' <= c && c <= 'Z') ||
                c == '_');
    }
};

class LatinUnderscoreDigitTransition : public LatinUnderscoreTransition
{
public:
    using LatinUnderscoreTransition::LatinUnderscoreTransition;

    bool isActive(char c) override
    {
        return LatinUnderscoreTransition::isActive(c) || ('0' <= c && c <= '9');
    }
};

template <class Transition> // TODO: put SFINAE here, requires 'bool Transition::isActive()'
class TransitionNegation : public Transition
{
public:
    bool isActive(char c) override
    {
        return !Transition::isActive(c);
    }
};

enum tokenCategory
{
    UNDEFINED,
    ONE,
    TWO,
    NAME,
};

class Node
{
public:
    Node(tokenCategory category = UNDEFINED)
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
    std::shared_ptr<Node> addTransitionToNewNode(tokenCategory category, TransitionArgs &&... args)
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
        return category_ != UNDEFINED;
    }

    tokenCategory getTokenCategory() const noexcept
    {
        return category_;
    }

private:
    std::vector<std::pair<std::unique_ptr<CharTransition>, std::shared_ptr<Node>>> transitions_;
    tokenCategory category_;
};

class Token
{
public:
    Token(tokenCategory category,
          std::string token,
          int position)
        : category_(std::move(category)),
          token_(std::move(token)),
          position_(std::move(position))
    {
    }

    tokenCategory getCategory() const noexcept
    {
        return category_;
    }

    std::string getToken() const noexcept
    {
        return token_;
    }

    int getPosition() const noexcept
    {
        return position_;
    }

private:
    tokenCategory category_;
    std::string token_;
    int position_;
};

class FiniteStateMachine
{
public:
    FiniteStateMachine()
        : root_(std::make_shared<Node>()),
          curNode_(root_)
    {
    }

    std::vector<std::unique_ptr<Token>> tokenizeText(const std::string &text)
    {
        std::vector<std::unique_ptr<Token>> tokens;
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
    std::unique_ptr<Token> followChar(char c)
    {
        auto nextNode_ = curNode_->getNextNode(c);
        std::unique_ptr<Token> result;
        if (nextNode_ && nextNode_->isTerminal())
        {
            auto tokenLen = curTokenString_.length();
            result = std::make_unique<Token>(curNode_->getTokenCategory(),
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

// Name here - pattern that starts with latin letter or underscore, contains latin letters, underscores and digits
class NameFinder : public FiniteStateMachine
{
public:
    NameFinder()
    {
        auto firstNode = getRoot()->addTransitionToNewNode<LatinUnderscoreTransition>(tokenCategory::UNDEFINED);
        firstNode->addTransition<LatinUnderscoreDigitTransition>(firstNode);
        firstNode->addTransitionToNewNode<TransitionNegation<LatinUnderscoreDigitTransition>>(tokenCategory::NAME);
        getRoot()->addTransition<TransitionNegation<LatinUnderscoreTransition>>(getRoot());
    }
};

int main()
{
    auto finder = NameFinder();
    for (const auto &token : finder.tokenizeText("This_is_name !1this1is_name_too___    "))
    {
        std::cout << token->getPosition() << " '" << token->getToken() << "'" << std::endl;
    }
}

/*

res = 4
i = 0
while (res != 0)
    i = i + 1
    res = res - 1
    if i % 2 == 0:
        res = res + a + b + i
    elif i % 2 == 1:
        res = 0
    else:
        res = res + c - i
res += c
*/
