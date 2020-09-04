#pragma once

#include <array>
#include <functional>
#include <memory>
#include <optional>
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

using ExprPtr = std::unique_ptr<Expression>;

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

template <typename T>
inline void checkOptionalNotEmptyOrThrow(const std::optional<T> &x,
                                         const std::string &message = "optional type is empty")
{
    if (!x.has_value())
    {
        throw std::invalid_argument("impossible to access object: " + message);
    }
}
} // namespace details

template <std::size_t Arity>
class FunctionCall : public Expression
{
public:
    using FunctionType = std::function<VType(details::TupleGenerator_t<VType, Arity>)>;
    constexpr static std::size_t arity = Arity;

    void setExpressions(const std::array<ExprPtr, Arity> &expressions) { expressions_.emplace(expressions); }
    void setExpressions(std::array<ExprPtr, Arity> &&expressions) { expressions_.emplace(std::move(expressions)); }

    void setFunction(const FunctionType &f) { f_.emplace(f); }
    void setFunction(FunctionType &&f) { f_.emplace(std::move(f)); }

    RvalueAndScope eval(Scope scope) override
    {
        details::checkOptionalNotEmptyOrThrow(expressions_, "expression is not set yet");
        details::checkOptionalNotEmptyOrThrow(f_, "function is not set yet");
        Scope currentScope = scope;
        details::TupleGenerator_t<VType, Arity> callArgs;
        std::size_t argumentId = 0;
        auto processSingleArgument = [&](VType &argument) {
            auto evalResult = expressions_.value()[argumentId++]->eval(currentScope);
            currentScope = evalResult.scope;
            argument = evalResult.value;
        };
        std::apply([&](auto &&... args) {
            (processSingleArgument(args), ...);
        },
                   callArgs);
        return {f_.value()(callArgs), currentScope};
    }

private:
    std::optional<std::array<ExprPtr, Arity>> expressions_;
    std::optional<FunctionType> f_;
};

using UnaryOperator = FunctionCall<1>;
using BinaryOperator = FunctionCall<2>;

class VTypeValue : public Expression
{
public:
    void setValue(VType value) { value_ = value; }

    RvalueAndScope eval(Scope scope) override
    {
        return {value_, scope};
    }

private:
    VType value_;
};

// class Conditional : public Expression
// {
// public:
//     struct CondWay
//     {
//         ExprPtr condition, way;
//     };

// public:
//     Conditional(CondWay ifWay,
//                 std::vector<CondWay> elifWay,
//                 ExprPtr elseWay)
//         : ifWay_(std::move(ifWay)),
//           elifWay_(std::move(elifWay)),
//           elseWay_(std::move(elseWay))
//     {
//     }

//     RvalueAndScope eval(Scope scope) override
//     {
//         if (!ifWay_.condition)
//         {
//             throw std::invalid_argument("if condition can not be empty");
//         }
//         if (ifWay_.condition->eval(scope).value)
//         {
//             if (ifWay_.way)
//             {
//                 ifWay_.way->eval(scope);
//             }
//             return {VType(), scope}; // TODO: wrong, because scope can't be changed
//         }
//         else
//         {
//             for (std::size_t elifTrue = 0, elifId = 0; !elifTrue && elifId < elifWay_.size(); elifId++)
//             {
//                 if (!elifWay_[elifId].condition)
//                 {
//                     throw std::invalid_argument("elif condition can not be empty");
//                 }
//                 elifTrue = elifWay_[elifId].condition->eval(scope).value;
//                 if (elifTrue)
//                 {
//                     if (elifWay_[elifId].way)
//                     {
//                         elifWay_[elifId].way->eval(scope);
//                     }
//                     return {VType(), scope};
//                 }
//             }
//         }
//         if (elseWay_)
//         {
//             elseWay_->eval(scope);
//         }
//         return {VType(), scope};
//     }

// private:
//     CondWay ifWay_;
//     std::vector<CondWay> elifWay_;
//     ExprPtr elseWay_;
// };

class ExpressionSequence : public Expression
{
public:
    void addExpression(ExprPtr &&expr)
    {
        exprs_.emplace_back(std::move(expr));
    }

    RvalueAndScope eval(Scope scope) override
    {
        VType result{};
        std::for_each(exprs_.begin(), exprs_.end(), [&](auto &expr) {
            result = expr->eval(scope).value;
        });
        return {result, scope};
    }

private:
    std::vector<ExprPtr> exprs_;
};
} // namespace expression
} // namespace peach
