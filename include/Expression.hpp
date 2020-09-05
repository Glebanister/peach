#pragma once

#include <array>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>

#include "Token.hpp"

namespace peach
{
namespace expression
{
// Variables type
using VType = std::int32_t; // TODO: wtf is this #1

// Scope is the current state of visible variables
using Scope = std::unordered_map<std::string, VType>; // TODO: wtf is this #2

class Expression;

using ExprPtr = std::unique_ptr<Expression>;
using ExprShPtr = std::shared_ptr<Expression>;

class Expression
{
public:
    // Evaluates all expression
    // Returns [evaluation result; state]
    virtual VType eval(Scope &) = 0;

    virtual void addExpressionFromNextIndentationLevel(ExprShPtr expr) = 0;

    virtual bool doesNeedBlockFromNextIndentationLevel() const noexcept = 0;

    virtual ~Expression() = default;
};

class SingleIndentationLevelExpression : public Expression
{
public:
    bool doesNeedBlockFromNextIndentationLevel() const noexcept final
    {
        return false;
    }

    void addExpressionFromNextIndentationLevel(ExprShPtr) final
    {
        throw std::invalid_argument("SingleIndentationLevelExpression can not be extended with expression from next indentation level");
    }
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
class FunctionCall : public SingleIndentationLevelExpression
{
public:
    using FunctionType = std::function<VType(details::TupleGenerator_t<VType, Arity>)>;
    constexpr static std::size_t arity = Arity;

    void setExpressions(const std::array<ExprShPtr, Arity> &expressions) { expressions_.emplace(expressions); }
    void setExpressions(std::array<ExprShPtr, Arity> &&expressions) { expressions_.emplace(std::move(expressions)); }

    void setFunction(const FunctionType &f) { f_.emplace(f); }
    void setFunction(FunctionType &&f) { f_.emplace(std::move(f)); }

    VType eval(Scope &scope) override
    {
        details::checkOptionalNotEmptyOrThrow(expressions_, "expression is not set yet");
        details::checkOptionalNotEmptyOrThrow(f_, "function is not set yet");
        details::TupleGenerator_t<VType, Arity> callArgs;
        std::size_t argumentId = 0;
        auto processSingleArgument = [&](VType &argument) {
            argument = expressions_.value()[argumentId++]->eval(scope);
        };
        std::apply([&](auto &&... args) {
            (processSingleArgument(args), ...);
        },
                   callArgs);
        return f_.value()(callArgs);
    }

private:
    std::optional<std::array<ExprShPtr, Arity>> expressions_;
    std::optional<FunctionType> f_;
};

using UnaryOperator = FunctionCall<1>;
using BinaryOperator = FunctionCall<2>;

class VTypeValue : public SingleIndentationLevelExpression
{
public:
    VTypeValue(VType v = 0)
        : value_(v) {}

    void setValue(VType value) { value_ = value; }

    VType eval(Scope &) override
    {
        return value_;
    }

private:
    VType value_;
};

class Conditional : public Expression
{
public:
    Conditional(ExprShPtr ifCond)
        : ifCond_(std::move(ifCond)) {}

    bool doesNeedBlockFromNextIndentationLevel() const noexcept final
    {
        return !ifWay_;
    }

    void addExpressionFromNextIndentationLevel(ExprShPtr expr) override
    {
        if (!ifWay_)
        {
            ifWay_ = std::move(expr);
        }
        else if (!elseWay_)
        {
            elseWay_ = std::move(expr);
        }
        else
        {
            throw std::invalid_argument("Conditional is already has if and else ways");
        }
    }

    VType eval(Scope &scope) override
    {
        if (ifCond_->eval(scope))
        {
            auto result = VType{};
            if (ifWay_)
            {
                result = ifWay_->eval(scope);
            }
            return result;
        }
        else if (elseWay_)
        {
            return elseWay_->eval(scope);
        }
        return VType{};
    }

private:
    ExprShPtr ifCond_, ifWay_, elseWay_;
};

class ExpressionSequence : public SingleIndentationLevelExpression
{
public:
    void addExpression(ExprShPtr expr)
    {
        exprs_.emplace_back(std::move(expr));
    }

    VType eval(Scope &scope) override
    {
        VType result{};
        std::for_each(exprs_.begin(), exprs_.end(), [&](auto &expr) {
            result = expr->eval(scope);
        });
        return result;
    }

private:
    std::vector<ExprShPtr> exprs_;
};

class LoopWhile : public Expression
{
public:
    LoopWhile(ExprShPtr cond)
        : loopCond_(std::move(cond)) {}

    bool doesNeedBlockFromNextIndentationLevel() const noexcept final
    {
        return true;
    }

    void addExpressionFromNextIndentationLevel(ExprShPtr expr) override
    {
        if (loopBody_)
        {
            throw std::invalid_argument("loop while already has body");
        }
        loopBody_ = expr;
    }

    VType eval(Scope &scope) override
    {
        VType result{};
        while (loopCond_->eval(scope))
        {
            result = loopBody_->eval(scope);
        }
        return result;
    }

private:
    ExprShPtr loopCond_;
    ExprShPtr loopBody_;
};

class AssignExpression : public SingleIndentationLevelExpression
{
public:
    AssignExpression(const std::string &name)
        : varName_(name) {}

    void setExpression(ExprShPtr expr) { expr_ = std::move(expr); }

    VType eval(Scope &scope) override
    {
        if (!expr_)
        {
            throw std::invalid_argument("AssignExpression does not have expression yet");
        }
        return scope[varName_] = expr_->eval(scope); // TODO: invisibility .........
    }

private:
    std::string varName_;
    ExprShPtr expr_;
};

class VariableAccess : public SingleIndentationLevelExpression
{
public:
    VariableAccess(const std::string &name)
        : varName_(name) {}

    VType eval(Scope &scope) override
    {
        return scope[varName_]; // TODO: undefined variable....
    }

private:
    std::string varName_;
};
} // namespace expression
} // namespace peach
