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

std::size_t indentation = 0;

struct Logger
{
    template <typename... Args>
    static void log(Args &&... args)
    {
        // for (std::size_t i = 0; i < indentation; ++i)
        // {
        //     std::cout << '\t';
        // }
        // (std::cout << ... << args);
        // std::cout.flush();
    }

    static void pushIndentation()
    {
        indentation++;
    }

    static void popIndentation()
    {
        indentation--;
    }
};

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
    VType &operator[](const std::string &varName)
    {
        auto it = memory_.find(varName);
        if (it == memory_.end())
        {
            exception::throwFromCoords<exception::UnknownVariable>(0, 0);
        }
        return it->second;
    }

    VType &declare(const std::string &varName, const VType &value = VType{})
    {
        return memory_[varName] = value;
    }

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

using PeachTuple = std::vector<VType>;

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

        Logger::log("function call begin, arguments:", '\n');
        Logger::pushIndentation();
        for (std::size_t argId = 0; argId < expressions_.value().size(); ++argId)
        {
            callArgs.emplace_back(expressions_.value()[argId]->eval(scope));
            Logger::log(callArgs.back(), '\n');
        }
        auto res = f_.value()(callArgs);
        Logger::popIndentation();
        Logger::log("function result: ", res, '\n');
        return res;
    }

private:
    std::optional<std::vector<ExprShPtr>> expressions_;
    std::optional<FunctionType> f_;
};

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
        while (auto res = loopCond_->eval(scope))
        {
            result = loopBody_->eval(scope);
        }
        return result;
    }

private:
    ExprShPtr loopCond_;
    ExprShPtr loopBody_;
};

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

class VariableAccess : public LvalueExpression
{
public:
    VariableAccess(const std::string &name)
        : LvalueExpression(name) {}

    VType eval(Scope &scope) override
    {
        if (!scope.hasName(varName_))
        {
            exception::throwFromCoords<exception::UnknownVariable>(111, 222); // Expression must know its position
        }
        Logger::log("access to variable ", varName_, " which is ", scope[varName_], '\n');
        return scope[varName_];
    }
};

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

class AssignExpression : public SingleIndentationLevelExpression
{
public:
    using FunctionType = std::function<VType(VType, VType)>;

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
        auto rightEvalRes = right_->eval(scope);
        Logger::log("assignation, value ", left_->getVariableName(), " before: ", scope[left_->getVariableName()], '\n');
        Logger::pushIndentation();
        auto res = functor_(scope[left_->getVariableName()], rightEvalRes);
        scope[left_->getVariableName()] = res;
        Logger::popIndentation();
        Logger::log("assignation, value ", left_->getVariableName(), " after: ", scope[left_->getVariableName()], '\n');
        return res;
    }

private:
    std::shared_ptr<LvalueExpression> left_;
    ExprShPtr right_;
    FunctionType functor_;
};
} // namespace expression
} // namespace peach
