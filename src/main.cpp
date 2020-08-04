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
        : begin_(begin), end_(end) {}

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

enum tokenCategory
{
    UNDEFINED,
    ONE,
    TWO,
};

class Node
{
public:
    Node(tokenCategory category = UNDEFINED)
        : category_(category) {}

    template <typename Transition>
    void addTransition(std::shared_ptr<Node> nextNode)
    {
        auto newTransition = std::make_unique<Transition>();
        transitions_.emplace_back(std::move(newTransition), std::move(nextNode));
    }

    template <typename Transition, typename... NodeArgs>
    std::shared_ptr<Node> addTransitionToNewNode(NodeArgs &&... args)
    {
        auto newNode = std::make_shared<Node>(std::forward<NodeArgs>(args)...);
        addTransition<Transition>(newNode);
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
        for (char c : text)
        {
            auto newToken = followChar(c);
            if (newToken)
            {
                tokens.emplace_back(std::move(newToken));
            }
        }
        return tokens;
    }

    std::shared_ptr<Node> getRoot() const
    {
        return root_;
    }

private:
    // Returns new Token pointer, if terminal node is reached, nullptr otherwise
    std::unique_ptr<Token> followChar(char c)
    {
        curNode_ = curNode_->getNextNode(c);
        std::unique_ptr<Token> result;
        curTokenString_ += c;
        ++curTokenPos_;
        if (curNode_ && curNode_->isTerminal())
        {
            auto tokenLen = curTokenString_.length();
            result = std::make_unique<Token>(curNode_->getTokenCategory(),
                                             std::move(curTokenString_),
                                             curTokenPos_ - tokenLen);
            resetToken();
        }
        else if (!curNode_) // TODO: may be not correct
        {
            resetToken();
        }
        return result;
    }

    void resetToken()
    {
        curNode_ = root_;
        curTokenString_.clear();
    }

    std::shared_ptr<Node> root_;
    std::shared_ptr<Node> curNode_;
    std::string curTokenString_ = "";
    int curTokenPos_ = 0;
};

class PrefixTree : public FiniteStateMachine
{
public:
    void addKeyword(const std::string &word)
    {
        auto curNode = getRoot();
        for (char c : word)
        {
            auto nextNode = curNode->getNextNode(c);
            if (!nextNode)
            {
                curNode->addTransitionToNewNode<SingleCharTransition<c>>()
            }
        }
    }
};

int main()
{
    auto machine = FiniteStateMachine();
    auto one = machine.getRoot()->addTransitionToNewNode<SingleCharTransition<'h'>>();
    auto two = one->addTransitionToNewNode<SingleCharTransition<'e'>>(tokenCategory::ONE);
    auto three = one->addTransitionToNewNode<SingleCharTransition<'i'>>(tokenCategory::TWO);

    auto result = machine.tokenizeText("heshrhehi");
    std::cout << result.size() << std::endl;
    for (auto &token : result)
    {
        std::cout << token->getPosition() << ' ' << token->getToken() << std::endl;
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
