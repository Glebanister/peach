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

template <char Begin, char End>
class RangeCharTransition : public CharTransition
{
public:
    bool isActive(char c) override
    {
        return Begin <= c && c <= End;
    }
};

template <char Char>
using SingleCharTransition = RangeCharTransition<Char, Char>;

template <char... Chars>
class SetCharTransition : public CharTransition
{
public:
    bool isActive(char c) override
    {
        bool result = false;
        ((result |= Chars == c), ...);
        return result;
    }
};

class Node
{
public:
    template <typename Transition, typename... TransitionArgs>
    void addTransition(std::shared_ptr<Node> nextNode, TransitionArgs &&... args)
    {
        auto newTransition = std::make_unique<Transition>(std::forward(args)...);
        transitions_.emplace_back(std::move(newTransition), std::move(nextNode));
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

private:
    std::vector<std::pair<std::unique_ptr<CharTransition>, std::shared_ptr<Node>>> transitions_;
};

template <typename... Args>
inline std::shared_ptr<Node> makeNode(Args &&... args)
{
    return std::make_shared<Node>(std::forward(args)...);
}

int main()
{
    auto node1 = makeNode();
    auto node2 = makeNode();
    node1->addTransition<SetCharTransition<'a'>>(node2);
    std::cout << node1->getNextNode('b') << std::endl;
}

/*

func foo(a, b, c):
    let res = 4
    let i = 0
    while (res != 0):
        i = i + 1
        res = res - 1
        if i % 2 == 0:
            res = res + a + b + i
        elif i % 2 == 1:
            res = 0
        else:
            res = res + c - i
    res = res + c
    return res
*/
