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
        reset();
        std::size_t curPrior = operators_.size() + 1;
        for (const auto &op : operators_)
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

    // Returns current indentation level
    std::size_t getIndentationLevel() const noexcept
    {
        return unfinishedExpressions_.size() - 1;
    }

    // Interpretates single line as Expression
    // Puts constructed expression to the corresponding place
    // Space tokens from begin to end sould not contain SEP_ENDL tokens
    void interpretateLine(TokenIterator beginTokens,
                          TokenIterator endTokens)
    {
        for (auto it = beginTokens; it < endTokens; ++it)
        {
            if (!(*it))
            {
                throw std::invalid_argument("can not interpretate nullptr token");
            }
            if (token::isEndline(*it))
            {
                throw std::invalid_argument("line can not contain endline token");
            }
            if ((*it)->getCategory() == token::tokenCategory::UNDEFINED)
            {
                exception::throwFromTokenIterator<exception::UndefinedTokenError>(it);
            }
        }
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
            if (getIndentationLevel() - lineIndentationLevel == 1 &&
                unfinishedExpressions_.top().type == indentationBlockType::COND_IF &&
                lineCategory == indentationBlockType::COND_ELSE)
            {
                unfinishedExpressions_.top().expression->addExpressionFromNextIndentationLevel(unfinishedExpressions_.top().sequence);
                unfinishedExpressions_.top().sequence = std::make_shared<expression::ExpressionSequence>();
                ++lineIndentationLevel;
                break;
            }
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

    // Interpreatates all given tokens to Expressions
    void interpretateLines(TokenIterator tokensBegin, TokenIterator tokensEnd)
    {
        while (tokensBegin < tokensEnd)
        {
            auto curEnd = getNextEndlineTokenIt(tokensBegin, tokensEnd);
            interpretateLine(tokensBegin, curEnd);
            tokensBegin = curEnd + 1;
        }
    }

    // Returns sequence of expressions built after interpretator construction or Interpreter::reset() call
    expression::ExprShPtr getInterpretationResult()
    {
        while (getIndentationLevel() > 0)
        {
            popIndentation();
        }
        return unfinishedExpressions_.top().sequence;
    }

    // Resets all built expressions
    void reset()
    {
        while (!unfinishedExpressions_.empty())
        {
            unfinishedExpressions_.pop();
        }
        pushNewIndentation(std::make_shared<expression::ExpressionSequence>(), indentationBlockType::UNDEFINED);
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

    static indentationBlockType getLineCategory(TokenIterator itBegin)
    {
        static_assert(static_cast<int>(indentationBlockType::UNDEFINED) == 0);
        auto itCategory = (*itBegin)->getCategory();
        static std::unordered_map<token::tokenCategory_t, indentationBlockType> blockTypeByToken = {
            {token::tokenCategory::COND_IF, indentationBlockType::COND_IF},
            {token::tokenCategory::COND_ELSE, indentationBlockType::COND_ELSE},
            {token::tokenCategory::LOOP_WHILE, indentationBlockType::LOOP_WHILE},
        };
        return blockTypeByToken[itCategory];
    }

    // Returns iterator on next non separation token
    static inline TokenIterator getNextNonSepTokenIt(TokenIterator begin, TokenIterator end)
    {
        for (; begin < end && token::isSeparator(*begin); ++begin)
            ;
        return begin;
    }

    // Returns iterator on next endline token
    static inline TokenIterator getNextEndlineTokenIt(TokenIterator begin, TokenIterator end)
    {
        for (; begin < end && !token::isEndline(*begin); ++begin)
            ;
        return begin;
    }

    // Constructs expression from line
    expression::ExprShPtr buildExpression(TokenIterator begin, TokenIterator end)
    {
        struct OperatorInfo
        {
            std::string string;
            token::tokenCategory_t category;
            TokenIterator position;
        };
        if (begin == end)
        {
            return std::make_shared<expression::VTypeValue>(0);
        }
        std::vector<OperatorInfo> operators;
        std::vector<expression::ExprShPtr> expressions;
        auto curIt = begin;

        auto throwBadOperator = [&]() {
            auto badIt = operators.empty() ? begin : operators.back().position;
            exception::throwFromTokenIterator<exception::SyntaxError>(badIt);
        };

        auto popUnaryOperator = [&]() {
            if (operators.empty() ||
                operators.back().category != token::tokenCategory::OPERATOR_UN ||
                expressions.empty())
            {
                throwBadOperator();
            }
            auto operatorExpression = std::make_shared<expression::UnaryOperator>();
            if (!unaryOperatorFunction_[operators.back().string])
            {
                throw std::invalid_argument("can not find unary operator " + operators.back().string);
            }
            operatorExpression->setFunction(unaryOperatorFunction_[operators.back().string]);
            operatorExpression->setExpressions({expressions.back()});
            operators.pop_back();
            expressions.pop_back();
            expressions.emplace_back(std::move(operatorExpression));
        };

        auto popBinaryOperator = [&]() {
            if (operators.empty() ||
                operators.back().category != token::tokenCategory::OPERATOR_BI ||
                expressions.size() < 2)
            {
                throwBadOperator();
            }
            auto operatorExpression = std::make_shared<expression::BinaryOperator>();
            if (!binaryOperatorFunction_[operators.back().string])
            {
                throw std::invalid_argument("can not find binary operator " + operators.back().string);
            }
            operatorExpression->setFunction(binaryOperatorFunction_[operators.back().string]);
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
                operators.back().category != token::tokenCategory::ASSIGNMENT ||
                expressions.size() < 2)
            {
                throwBadOperator();
            }
            std::shared_ptr<expression::AssignExpression> assignExpr;
            std::shared_ptr<expression::Expression> rightExpr;
            rightExpr = expressions.back();
            expressions.pop_back();
            assignExpr = std::dynamic_pointer_cast<expression::AssignExpression>(expressions.back());
            if (!assignExpr)
            {
                exception::throwFromTokenIterator<exception::SyntaxError>(operators.back().position);
            }
            assignExpr->setExpression(rightExpr);
            expressions.pop_back();
            operators.pop_back();
            expressions.emplace_back(std::move(assignExpr));
        };

        auto popOperator = [&]() {
            if (operators.empty())
            {
                throwBadOperator();
            }
            switch (operators.back().category)
            {
            case token::tokenCategory::OPERATOR_UN:
                popUnaryOperator();
                break;
            case token::tokenCategory::OPERATOR_BI:
                popBinaryOperator();
                break;
            case token::tokenCategory::ASSIGNMENT:
                popAssignment();
                break;
            case token::tokenCategory::BRACKET_OPEN:
                exception::throwFromTokenIterator<exception::BracketDisbalanceError>(operators.back().position);
            default:
                exception::throwFromTokenIterator<exception::UndefinedOperatorError>(operators.back().position);
            }
        };

        auto pushOperator = [&](const OperatorInfo &op) {
            if (!operators.empty())
            {
                while (!operators.empty() && getOperatorPriority(operators.back().string) > getOperatorPriority(op.string))
                {
                    popOperator();
                }
            }
            operators.push_back(op);
        };

        for (; curIt < end; curIt = getNextNonSepTokenIt(curIt + 1, end))
        {
            auto &token = *curIt;

            switch (token->getCategory())
            {
            case token::tokenCategory::VALUE_INT:
                expressions.push_back(std::make_shared<expression::VTypeValue>(std::stoi(token->getTokenString())));
                break;

            case token::tokenCategory::NAME:
                if (curIt + 1 < end && (*(getNextNonSepTokenIt(curIt + 1, end)))->getCategory() == token::tokenCategory::ASSIGNMENT)
                {
                    auto str = token->getTokenString();
                    expressions.push_back(std::make_shared<expression::AssignExpression>(token->getTokenString()));
                }
                else
                {
                    expressions.push_back(std::make_shared<expression::VariableAccess>(token->getTokenString()));
                }
                break;

            case token::tokenCategory::ASSIGNMENT:
            case token::tokenCategory::OPERATOR_UN:
            case token::tokenCategory::OPERATOR_BI:
                pushOperator({token->getTokenString(), token->getCategory(), curIt});
                break;

            case token::tokenCategory::BRACKET_OPEN:
                operators.push_back({token->getTokenString(), token->getCategory(), curIt});
                break;

            case token::tokenCategory::BRACKET_CLOSE:
                while (!operators.empty() && operators.back().category != token::tokenCategory::BRACKET_OPEN)
                {
                    popOperator();
                }
                if (operators.empty())
                {
                    exception::throwFromTokenIterator<exception::BracketDisbalanceError>(curIt);
                }
                operators.pop_back();
                break;

            default:
                exception::throwFromTokenIterator<exception::UnexpectedTokenError>(curIt);
                break;
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
        if (op == "(")
        {
            return 0;
        }
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
