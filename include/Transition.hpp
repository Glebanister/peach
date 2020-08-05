#pragma once

#include <algorithm>
#include <initializer_list>
#include <vector>

namespace peach
{
namespace transition
{
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

} // namespace transition
} // namespace peach
