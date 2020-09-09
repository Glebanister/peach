#pragma once

#include <array>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>

#include "Exception.hpp"
#include "Token.hpp"

namespace peach
{
namespace expression
{
// Variables type
using VType = std::int32_t; // TODO: literals at least

// Scope is the current state of visible variables
class Scope
{
public:
    // Returns VType that corresponds to varName
    // If scope does not contain such varName, throws UnknownVariableError
    VType &operator[](const std::string &varName)
    {
        auto it = memory_.find(varName);
        if (it == memory_.end())
        {
            exception::throwFromCoords<exception::UnknownVariableError>(0, 0);
        }
        return it->second;
    }

    // Declares variable varName in scope, value can be presetted
    // Returns reference on varName in scope
    VType &declare(const std::string &varName, const VType &value = VType{})
    {
        return memory_[varName] = value;
    }

    // Returns if scope contains variable with name varName
    bool hasName(const std::string &varName) const
    {
        return memory_.find(varName) != memory_.end();
    }

private:
    std::unordered_map<std::string, VType> memory_;
};

class Expression;

using ExprPtr = std::unique_ptr<Expression>;
using ExprShPtr = std::shared_ptr<Expression>;

// Object, that can be evaluated
class Expression
{
public:
    // Evaluates all expression
    // Returns [evaluation result; state]
    virtual VType eval(Scope &) = 0;

    //
    virtual void addExpressionFromNextIndentationLevel(ExprShPtr expr) = 0;

    virtual ~Expression() = default;
};

// Expression that does not need next level indentation block
// For example: 'if' or 'while' does need it, 'let a = 0' does not
class SingleIndentationLevelExpression : public Expression
{
public:
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

using PeachTuple = std::vector<VType>;

// Any function or operator call in peach
class FunctionCall : public SingleIndentationLevelExpression
{
public:
    using FunctionType = std::function<VType(PeachTuple)>;

    void setExpressions(const std::vector<ExprShPtr> &expressions) { expressions_.emplace(expressions); }
    void setExpressions(std::vector<ExprShPtr> &&expressions) { expressions_.emplace(std::move(expressions)); }

    void setFunction(const FunctionType &f) { f_.emplace(f); }
    void setFunction(FunctionType &&f) { f_.emplace(std::move(f)); }

    VType eval(Scope &scope) override
    {
        details::checkOptionalNotEmptyOrThrow(expressions_, "expression is not set yet");
        details::checkOptionalNotEmptyOrThrow(f_, "function is not set yet");

        PeachTuple callArgs;
        for (std::size_t argId = 0; argId < expressions_.value().size(); ++argId)
        {
            callArgs.emplace_back(expressions_.value()[argId]->eval(scope));
        }
        auto res = f_.value()(callArgs);
        return res;
    }

private:
    std::optional<std::vector<ExprShPtr>> expressions_;
    std::optional<FunctionType> f_;
};

// Value of type VType
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

// if/else conditional. if ifCond_ is true, evaluates ifWay_, elseWay_ otherwise
class Conditional : public Expression
{
public:
    Conditional(ExprShPtr ifCond)
        : ifCond_(std::move(ifCond)) {}

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

// Sequence of Expressions, evaluates in adding order
class ExpressionSequence : public SingleIndentationLevelExpression
{
public:
    // Adds expr to sequence
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

// while loop
// while loopCond_ returns true loopBody_ evaluates
class LoopWhile : public Expression
{
public:
    LoopWhile(ExprShPtr cond)
        : loopCond_(std::move(cond)) {}

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

// Expression that contains variable
class LvalueExpression : public SingleIndentationLevelExpression
{
public:
    LvalueExpression(const std::string &name)
        : varName_(name) {}

    std::string getVariableName() const noexcept
    {
        return varName_;
    }

protected:
    std::string varName_;
};

// Gives access to variable with name 'name'
class VariableAccess : public LvalueExpression
{
public:
    VariableAccess(const std::string &name)
        : LvalueExpression(name) {}

    VType eval(Scope &scope) override
    {
        if (!scope.hasName(varName_))
        {
            exception::throwFromCoords<exception::UnknownVariableError>(111, 222); // Expression must know its position
        }
        return scope[varName_];
    }
};

// Declares variable with name 'name'
class VariableDeclaration : public LvalueExpression
{
public:
    VariableDeclaration(const std::string &name)
        : LvalueExpression(name) {}

    VType eval(Scope &scope) override
    {
        if (scope.hasName(varName_))
        {
            exception::throwFromCoords<exception::VariableRedeclaration>(0, 0); // Expression must know its position
        }
        return scope.declare(varName_);
    }
};

// Assigns left expression to right.
// Can be constructed iff left expression is LvalueExpression
class AssignExpression : public SingleIndentationLevelExpression
{
public:
    using FunctionType = std::function<void(VType &, VType)>;

    AssignExpression(
        ExprShPtr left,
        ExprShPtr right,
        FunctionType functor)
        : functor_(std::move(functor))
    {
        if (!left || !right)
        {
            throw std::invalid_argument("nullptr expression in AssignExpression constructor");
        }
        left_ = std::dynamic_pointer_cast<LvalueExpression>(left);
        if (!left_)
        {
            exception::throwFromCoords<exception::InvalidAssignationError>(0, 0); // TODO: expression must know its position
        }
        right_ = std::move(right);
    }

    VType eval(Scope &scope) override
    {
        if (!left_ || !right_)
        {
            throw std::invalid_argument("AssignExpression does not have expression yet");
        }
        VType rightEvalRes = right_->eval(scope);
        VType &leftVariable = scope[left_->getVariableName()];
        functor_(leftVariable, rightEvalRes);
        return leftVariable;
    }

private:
    std::shared_ptr<LvalueExpression> left_;
    ExprShPtr right_;
    FunctionType functor_;
};
} // namespace expression
} // namespace peach
