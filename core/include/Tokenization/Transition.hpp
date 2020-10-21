#pragma once

#include <algorithm>
#include <tuple>
#include <vector>

namespace peach
{
namespace transition
{
// Transition for FiniteStateMachine
class CharTransition
{
public:
    // Returns if transition is active on recieved char
    virtual bool isActive(char) = 0;
    virtual ~CharTransition() = default;
};

// Always true transition
class TrueTransition : public CharTransition
{
public:
    bool isActive(char) override
    {
        return true;
    }
};

// Merge of TransitionClasses
// Active iff at least one of TransitionClasses is active
// TransitionClasses must be heir of CharTransition
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

// Transition that is active iff char is in range from begin to end including
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

// Transition that is active iff char is equiv
class SingleCharTransition : public RangeCharTransition
{
public:
    SingleCharTransition(char c)
        : RangeCharTransition(c, c) {}
};

// Transition that is active iff char is in charset
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

// Template for range transition
template <char Begin, char End>
class RangeCharTransitionTemplate : public RangeCharTransition
{
public:
    RangeCharTransitionTemplate()
        : RangeCharTransition(Begin, End) {}
};

// Template for SingleCharTransition
template <char C>
class SingleCharTransitionTemplate : public RangeCharTransition
{
public:
    SingleCharTransitionTemplate()
        : RangeCharTransition(C, C) {}
};

// Transition that is active iff Transition is not active
template <class Transition> // TODO: put SFINAE here, requires 'bool Transition::isActive()'
class TransitionNegation : public Transition
{
public:
    bool isActive(char c) override
    {
        return !Transition::isActive(c);
    }
};

// Following transitions are active iff:
using DigitCharTransition = RangeCharTransitionTemplate<'0', '9'>;                                       // '0' - '9'
using LowerLatinCharTransition = RangeCharTransitionTemplate<'a', 'z'>;                                  // 'a' - 'z'
using UpperLatinCharTransition = RangeCharTransitionTemplate<'A', 'Z'>;                                  // 'A' - 'Z'
using LatinCharTransition = MergeTransitions<LowerLatinCharTransition, UpperLatinCharTransition>;        // 'a' - 'z' | 'A' - 'Z'
using UnderscoreTransition = SingleCharTransitionTemplate<'_'>;                                          // '_'
using LatinUnderscoreTransition = MergeTransitions<LatinCharTransition, UnderscoreTransition>;           // 'a' - 'z' | 'A' - 'Z' | '_'
using LatinUnderscoreDigitTransition = MergeTransitions<LatinUnderscoreTransition, DigitCharTransition>; // 'a' - 'z' | 'A' - 'Z' | '0' - '9'
using FalseTransition = TransitionNegation<TrueTransition>;                                              // never

namespace transitionShortcuts
{
using digit = DigitCharTransition;
using lower = LowerLatinCharTransition;
using upper = UpperLatinCharTransition;
using letter = LatinCharTransition;
using alphaNum = LatinUnderscoreDigitTransition;
using fls = FalseTransition;
using tru = TrueTransition;
} // namespace transitionShortcuts
} // namespace transition
} // namespace peach
