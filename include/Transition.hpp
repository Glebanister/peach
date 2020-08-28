#pragma once

#include <algorithm>
#include <tuple>
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

class TrueTransition : public CharTransition
{
public:
    bool isActive(char) override
    {
        return true;
    }
};

template <typename... TransitionClasses> // TODO: sfinae for default-construction check
class MergeTransitions : public CharTransition
{
public:
    bool isActive(char c) override
    {
        return std::apply(
            [&](TransitionClasses &... transitions) {
                return (... || transitions.isActive(c));
            },
            subtransitions_);
    }

private:
    std::tuple<TransitionClasses...> subtransitions_;
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
    SetCharTransition(std::vector<char> charset)
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

template <char C>
class SingleCharTransitionTemplate : public RangeCharTransition
{
public:
    SingleCharTransitionTemplate()
        : RangeCharTransition(C, C) {}
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

using DigitCharTransition = RangeCharTransitionTemplate<'0', '9'>;
using LowerLatinCharTransition = RangeCharTransitionTemplate<'a', 'z'>;
using UpperLatinCharTransition = RangeCharTransitionTemplate<'A', 'Z'>;
using LatinCharTransition = MergeTransitions<LowerLatinCharTransition, UpperLatinCharTransition>;
using UnderscoreTransition = SingleCharTransitionTemplate<'_'>;
using LatinUnderscoreTransition = MergeTransitions<LatinCharTransition, UnderscoreTransition>;
using LatinUnderscoreDigitTransition = MergeTransitions<LatinUnderscoreTransition, DigitCharTransition>;
using FalseTransition = TransitionNegation<TrueTransition>;

} // namespace transition
} // namespace peach
