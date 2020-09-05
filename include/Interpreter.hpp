#pragma once

#include <stack>
#include <variant>

#include "Exception.hpp"
#include "Expression.hpp"
#include "Indentator.hpp"
#include "Token.hpp"

namespace peach
{
namespace interpreter
{
template <std::size_t Arity>
struct OperatorInfo
{
    std::function<expression::VType(expression::details::TupleGenerator_t<expression::VType, Arity>)> functor;
    std::string tokenString;
    token::tokenCategory_t tokenCategory;
};

using UnaryOperatorInfo = OperatorInfo<1>;
using BinaryOperatorInfo = OperatorInfo<2>;

struct AssignationInfo
{
    std::string tokenString;
};

// Interpretates tokens into evaluable Exresstions
class Interpreter
{
private:
    enum class indentationBlockType
    {
        UNDEFINED,
        LOOP_WHILE,
        COND_IF,
        COND_ELSE,
    };

    struct UnfinishedExpression
    {
        expression::ExprShPtr expression;
        std::shared_ptr<expression::ExpressionSequence> sequence;
        indentationBlockType type;
    };

public:
    Interpreter(std::vector<token::tokenCategory_t> singleIndentationBlock,
                std::vector<std::variant<UnaryOperatorInfo, BinaryOperatorInfo, AssignationInfo>> operatorsOrder)
        : singleIndentationBlock_(std::move(singleIndentationBlock)),
          operators_(std::move(operatorsOrder))
    {
        pushNewIndentation(std::make_shared<expression::ExpressionSequence>(), indentationBlockType::UNDEFINED);
        std::size_t curPrior = operators_.size() + 1;
        for (const std::variant<UnaryOperatorInfo, BinaryOperatorInfo, AssignationInfo> &op : operators_)
        {
            if (op.index() == 0) // UnaryOperator
            {
                auto curop = std::get<0>(op);
                operatorPriority_[curop.tokenString] = curPrior;
                unaryOperatorFunction_[curop.tokenString] = curop.functor;
            }
            else if (op.index() == 1) // BinaryOperator
            {
                auto curop = std::get<1>(op);
                operatorPriority_[curop.tokenString] = curPrior;
                binaryOperatorFunction_[curop.tokenString] = curop.functor;
            }
            else // Assign
            {
                auto curop = std::get<2>(op);
                operatorPriority_[curop.tokenString] = curPrior;
            }

            curPrior--;
        }
    }

    std::size_t getIndentationLevel() const noexcept
    {
        return unfinishedExpressions_.size() - 1;
    }

    // Interpretates single line as Expression
    // Puts constructed expression to the corresponding place
    void interpretateLine(TokenIterator beginTokens,
                          TokenIterator endTokens)
    {
        if (unfinishedExpressions_.empty())
        {
            throw std::logic_error("unfinished expressions stack is empty");
        }
        auto [lineIndentationLevel, firstNotIndentationIt] = Indentator::getIndentation(beginTokens,
                                                                                        endTokens,
                                                                                        singleIndentationBlock_);
        beginTokens = firstNotIndentationIt;
        auto lineCategory = getLineCategory(beginTokens);

        while (lineIndentationLevel < getIndentationLevel())
        {
            popIndentation();
        }

        if (lineIndentationLevel != getIndentationLevel())
        {
            exception::throwFromTokenIterator<exception::IndentationError>(beginTokens);
        }
        if (lineCategory == indentationBlockType::COND_IF)
        {
            auto newConditionalExpression = std::make_shared<expression::Conditional>(
                buildExpression(
                    getNextNonSepTokenIt(beginTokens + 1, endTokens), endTokens));
            pushNewIndentation(newConditionalExpression, lineCategory);
        }
        else if (lineCategory == indentationBlockType::COND_ELSE)
        {
            if (unfinishedExpressions_.top().type != indentationBlockType::COND_IF)
            {
                exception::throwFromTokenIterator<exception::UnexpectedElseError>(beginTokens);
            }
            unfinishedExpressions_.top().expression->addExpressionFromNextIndentationLevel(unfinishedExpressions_.top().sequence);
            unfinishedExpressions_.top().sequence = std::make_shared<expression::ExpressionSequence>();
        }
        else if (lineCategory == indentationBlockType::LOOP_WHILE)
        {
            auto newLoopExpression = std::make_shared<expression::LoopWhile>(
                buildExpression(
                    getNextNonSepTokenIt(beginTokens + 1, endTokens), endTokens));
            pushNewIndentation(newLoopExpression, lineCategory);
        }
        else
        {
            unfinishedExpressions_.top().sequence->addExpression(buildExpression(beginTokens, endTokens));
        }
    }

    expression::ExprShPtr getInterpretationResult() &&
    {
        return unfinishedExpressions_.top().sequence;
    }

private:
    void popIndentation()
    {
        auto expr = unfinishedExpressions_.top().expression;
        expr->addExpressionFromNextIndentationLevel(unfinishedExpressions_.top().sequence);
        unfinishedExpressions_.pop();
        unfinishedExpressions_.top().sequence->addExpression(expr);
    }

    void pushNewIndentation(expression::ExprShPtr expr, indentationBlockType type)
    {
        UnfinishedExpression newExpr;
        newExpr.expression = std::move(expr);
        newExpr.sequence = std::make_shared<expression::ExpressionSequence>();
        newExpr.type = type;
        unfinishedExpressions_.emplace(std::move(newExpr));
    }

    indentationBlockType getLineCategory(TokenIterator itBegin)
    {
        static_assert(static_cast<int>(indentationBlockType::UNDEFINED) == 0);
        auto itCategory = (*itBegin)->getCategory();
        static std::unordered_map<token::tokenCategory_t, indentationBlockType> blockTypeByToken = {
            {token::tokenCategory::COND_IF, indentationBlockType::COND_IF},
            {token::tokenCategory::COND_ELSE, indentationBlockType::COND_ELSE},
        };
        return blockTypeByToken[itCategory];
    }

    // Returns iterator on next non separation token
    TokenIterator getNextNonSepTokenIt(TokenIterator begin, TokenIterator end)
    {
        while (begin < end && token::isSeparator(*begin))
        {
            ++begin;
        }
        return begin;
    }

    // Constructs expression from line
    expression::ExprShPtr buildExpression(TokenIterator begin, TokenIterator end)
    {
        std::vector<std::pair<std::string, token::tokenCategory_t>> operators;
        std::vector<expression::ExprShPtr> expressions;
        auto curIt = begin;

        auto popUnaryOperator = [&]() {
            if (operators.empty() ||
                operators.back().second != token::tokenCategory::OPERATOR_UN ||
                expressions.empty())
            {
                throw std::invalid_argument("can not pop unary operator");
            }
            auto operatorExpression = std::make_shared<expression::UnaryOperator>();
            if (!unaryOperatorFunction_[operators.back().first])
            {
                throw std::invalid_argument("can not find unary operator " + operators.back().first);
            }
            operatorExpression->setFunction(unaryOperatorFunction_[operators.back().first]);
            operatorExpression->setExpressions({expressions.back()});
            operators.pop_back();
            expressions.pop_back();
            expressions.emplace_back(std::move(operatorExpression));
        };

        auto popBinaryOperator = [&]() {
            if (operators.empty() ||
                operators.back().second != token::tokenCategory::OPERATOR_BI ||
                expressions.size() < 2)
            {
                throw std::invalid_argument("can not pop binary operator");
            }
            auto operatorExpression = std::make_shared<expression::BinaryOperator>();
            if (!binaryOperatorFunction_[operators.back().first])
            {
                throw std::invalid_argument("can not find binary operator " + operators.back().first);
            }
            operatorExpression->setFunction(binaryOperatorFunction_[operators.back().first]);
            auto firstExpr = expressions.back();
            expressions.pop_back();
            auto secondExpr = expressions.back();
            expressions.pop_back();
            operatorExpression->setExpressions({secondExpr, firstExpr});
            operators.pop_back();
            expressions.emplace_back(std::move(operatorExpression));
        };

        auto popAssignment = [&]() {
            if (operators.empty() ||
                operators.back().second != token::tokenCategory::ASSIGNMENT ||
                expressions.size() < 2)
            {
                throw std::invalid_argument("can not pop assignment operator");
            }
            std::shared_ptr<expression::AssignExpression> assignExpr;
            std::shared_ptr<expression::Expression> rightExpr;
            rightExpr = expressions.back();
            expressions.pop_back();
            assignExpr = std::dynamic_pointer_cast<expression::AssignExpression>(expressions.back());
            if (!assignExpr)
            {
                exception::throwFromTokenIterator<exception::SyntaxError>(begin);
            }
            assignExpr->setExpression(rightExpr);
            expressions.pop_back();
            operators.pop_back();
            expressions.emplace_back(std::move(assignExpr));
        };

        auto popOperator = [&]() {
            if (operators.empty())
            {
                throw std::invalid_argument("can not pop operator from stack");
            }
            if (operators.back().second == token::tokenCategory::OPERATOR_UN)
            {
                popUnaryOperator();
            }
            else if (operators.back().second == token::tokenCategory::OPERATOR_BI)
            {
                popBinaryOperator();
            }
            else if (operators.back().second == token::tokenCategory::ASSIGNMENT)
            {
                popAssignment();
            }
        };

        auto pushOperator = [&](const std::pair<std::string, token::tokenCategory_t> &op) {
            if (!operators.empty())
            {
                auto topOperator = operators.back();
                while (!operators.empty() && getOperatorPriority(operators.back().first) > getOperatorPriority(op.first))
                {
                    popOperator();
                }
            }
            operators.push_back(op);
        };

        for (; curIt < end; curIt = getNextNonSepTokenIt(curIt + 1, end))
        {
            auto &token = *curIt;
            if (token->getCategory() == token::tokenCategory::VALUE_INT)
            {
                expressions.push_back(std::make_shared<expression::VTypeValue>(std::stoi(token->getTokenString())));
            }
            else if (token->getCategory() == token::tokenCategory::NAME)
            {
                if (curIt + 1 != end && (*(getNextNonSepTokenIt(curIt + 1, end)))->getCategory() == token::tokenCategory::ASSIGNMENT)
                {
                    auto str = token->getTokenString();
                    expressions.push_back(std::make_shared<expression::AssignExpression>(token->getTokenString()));
                }
                else
                {
                    expressions.push_back(std::make_shared<expression::VariableAccess>(token->getTokenString()));
                }
            }
            else if (token->getCategory() == token::tokenCategory::ASSIGNMENT)
            {
                pushOperator({token->getTokenString(), token->getCategory()});
            }
            else if (token->getCategory() == token::tokenCategory::OPERATOR_UN ||
                     token->getCategory() == token::tokenCategory::OPERATOR_BI)
            {
                pushOperator({token->getTokenString(), token->getCategory()});
            }
            else
            {
                exception::throwFromTokenIterator<exception::SyntaxError>(curIt);
            }
            if (operators.empty())
            {
                if (expressions.size() != 1)
                {
                    exception::throwFromTokenIterator<exception::SyntaxError>(curIt);
                }
            }
        }
        while (!operators.empty())
        {
            popOperator();
        }
        if (expressions.size() != 1)
        {
            exception::throwFromTokenIterator<exception::SyntaxError>(begin);
        }
        return expressions[0];
    }

    int getOperatorPriority(const std::string &op)
    {
        if (!operatorPriority_[op])
        {
            throw std::invalid_argument("undefined operator " + op);
        }
        return operatorPriority_[op];
    }

    std::vector<token::tokenCategory_t> singleIndentationBlock_;
    std::stack<UnfinishedExpression> unfinishedExpressions_;
    std::vector<std::variant<UnaryOperatorInfo, BinaryOperatorInfo, AssignationInfo>> operators_;
    std::unordered_map<std::string, int> operatorPriority_;
    std::unordered_map<std::string, std::function<expression::VType(std::tuple<expression::VType>)>> unaryOperatorFunction_;
    std::unordered_map<std::string, std::function<expression::VType(std::tuple<expression::VType, expression::VType>)>> binaryOperatorFunction_;
}; // namespace interpreter
} // namespace interpreter
} // namespace peach
