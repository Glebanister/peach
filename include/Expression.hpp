#pragma once

#include <array>
#include <functional>
#include <memory>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>

namespace peach
{
namespace expression
{
// Variables type
using VType = std::int32_t; // TODO: wtf is this #1

// Scope is the current state of visible variables
using Scope = std::unordered_map<std::string, VType>; // TODO: wtf is this #2

struct RvalueAndScope
{
    VType value;
    Scope scope;
};

class Expression
{
public:
    virtual RvalueAndScope eval(Scope) = 0;
    virtual ~Expression() = default;
};

namespace details
{
// Function return type correct iff it returns VType
template <typename Function, typename... Args>
struct IsFunctionRetTypeCorrect
{
    constexpr static bool value = std::is_same_v<std::invoke_result_t<Function, Args...>, VType>;
};

// Generates tuple of T of length N
template <typename T, std::size_t N, typename... Tail>
struct TupleGenerator
{
    using type = typename TupleGenerator<T, N - 1, T, Tail...>::type;
};

template <typename T, typename... Tail>
struct TupleGenerator<T, 0, Tail...>
{
    using type = std::tuple<Tail...>;
};

template <typename T, std::size_t N>
using TupleGenerator_t = typename TupleGenerator<T, N>::type;
} // namespace details

template <std::size_t Arity>
class FunctionCall : public Expression
{
public:
    using FunctionType = std::function<VType(details::TupleGenerator_t<VType, Arity>)>;
    constexpr static std::size_t arity = Arity;

    FunctionCall(std::array<std::unique_ptr<Expression>, Arity> &&expressions,
                 FunctionType f)
        : expressions_(std::move(expressions)), f_(std::move(f)) {}

    RvalueAndScope eval(Scope scope) override
    {
        Scope currentScope = scope;
        details::TupleGenerator_t<VType, Arity> callArgs;
        std::size_t argumentId = 0;
        auto processSingleArgument = [&](VType &argument) {
            auto evalResult = expressions_[argumentId++]->eval(currentScope);
            currentScope = evalResult.scope;
            argument = evalResult.value;
        };
        std::apply([&](auto &&... args) {
            (processSingleArgument(args), ...);
        },
                   callArgs);
        return {f_(callArgs), currentScope};
    }

private:
    std::array<std::unique_ptr<Expression>, Arity> expressions_;
    FunctionType f_;
};

using UnaryOperator = FunctionCall<1>;
using BinaryOperator = FunctionCall<2>;

class VTypeValue : public Expression
{
public:
    VTypeValue(VType value)
        : value_(std::move(value)) {}

    RvalueAndScope eval(Scope scope) override
    {
        return {value_, scope};
    }

private:
    VType value_;
};
} // namespace expression
} // namespace peach
