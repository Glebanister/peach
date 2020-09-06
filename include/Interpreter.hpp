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
struct OperatorInfo
{
    expression::FunctionCall::FunctionType functor;
    std::string tokenString;
    token::tokenCategory_t tokenCategory;
};

struct AssignOperatorInfo
{
    expression::AssignExpression::FunctionType functor;
    std::string tokenString;
    token::tokenCategory_t tokenCategory;
};

// Interpretates tokens into evaluable Exresstions
class Interpreter
{
private:
    struct UnfinishedExpression
    {
        expression::ExprShPtr expression;
        std::shared_ptr<expression::ExpressionSequence> sequence;
        token::tokenCategory_t type;
    };

public:
    Interpreter(std::vector<token::tokenCategory_t> singleIndentationBlock,
                std::vector<OperatorInfo> operatorsOrder,
                std::vector<AssignOperatorInfo> assignOperatorsOrder)
        : singleIndentationBlock_(std::move(singleIndentationBlock))
    {
        reset();
        std::size_t curPrior = assignOperatorsOrder.size() + 1 + operatorsOrder.size();
        for (const auto &op : operatorsOrder)
        {
            operatorPriority_[op.tokenString] = curPrior--;
            operatorFunction_[op.tokenString] = op.functor;
        }
        for (const auto &op : assignOperatorsOrder)
        {
            operatorPriority_[op.tokenString] = curPrior--;
            assignOperatorFunction_[op.tokenString] = op.functor;
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
        if (beginTokens == endTokens)
        {
            return;
        }
        auto lineCategory = getLineCategory(beginTokens);

        while (lineIndentationLevel < getIndentationLevel())
        {
            if (getIndentationLevel() - lineIndentationLevel == 1 &&
                unfinishedExpressions_.top().type == token::tokenCategory::COND_IF &&
                lineCategory == token::tokenCategory::COND_ELSE)
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
        switch (lineCategory)
        {
        case token::tokenCategory::COND_IF:
        {
            auto newConditionalExpression = std::make_shared<expression::Conditional>(
                buildExpression(
                    getNextNonSepTokenIt(beginTokens + 1, endTokens), endTokens));
            pushNewIndentation(newConditionalExpression, lineCategory);
            break;
        }

        case token::tokenCategory::COND_ELSE:
        {
            if (unfinishedExpressions_.top().type != token::tokenCategory::COND_IF)
            {
                exception::throwFromTokenIterator<exception::UnexpectedElseError>(beginTokens);
            }
            break;
        }

        case token::tokenCategory::LOOP_WHILE:
        {
            auto newLoopExpression = std::make_shared<expression::LoopWhile>(
                buildExpression(
                    getNextNonSepTokenIt(beginTokens + 1, endTokens), endTokens));
            pushNewIndentation(newLoopExpression, lineCategory);
            break;
        }

        case token::tokenCategory::DECLARATION:
        {
            auto nameIterator = getNextNonSepTokenIt(beginTokens + 1, endTokens);
            if (!(*nameIterator) || (*nameIterator)->getCategory() != token::tokenCategory::NAME)
            {
                exception::throwFromTokenIterator<exception::InvalidVariableDeclarationError>(nameIterator);
            }

            expression::ExprShPtr declaration = std::make_shared<expression::VariableDeclaration>((*nameIterator)->getTokenString());
            expression::ExprShPtr definition = buildExpression(nameIterator, endTokens);
            unfinishedExpressions_.top().sequence->addExpression(std::move(declaration));
            unfinishedExpressions_.top().sequence->addExpression(std::move(definition));
            break;
        }

        default:
            unfinishedExpressions_.top().sequence->addExpression(buildExpression(beginTokens, endTokens));
            break;
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
        pushNewIndentation(std::make_shared<expression::ExpressionSequence>(), token::tokenCategory::UNDEFINED);
    }

private:
    void popIndentation()
    {
        auto expr = unfinishedExpressions_.top().expression;
        expr->addExpressionFromNextIndentationLevel(unfinishedExpressions_.top().sequence);
        unfinishedExpressions_.pop();
        unfinishedExpressions_.top().sequence->addExpression(expr);
    }

    void pushNewIndentation(expression::ExprShPtr expr, token::tokenCategory_t type)
    {
        UnfinishedExpression newExpr;
        newExpr.expression = std::move(expr);
        newExpr.sequence = std::make_shared<expression::ExpressionSequence>();
        newExpr.type = type;
        unfinishedExpressions_.emplace(std::move(newExpr));
    }

    static token::tokenCategory_t getLineCategory(TokenIterator itBegin)
    {
        return (*itBegin)->getCategory();
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

        auto popArOperator = [&](std::size_t arity) {
            if (operators.empty() ||
                expressions.size() < arity)
            {
                throwBadOperator();
            }
            auto operatorExpression = std::make_shared<expression::FunctionCall>();
            if (!operatorFunction_[operators.back().string])
            {
                throw std::invalid_argument("can not find operator " + operators.back().string);
            }
            operatorExpression->setFunction(operatorFunction_[operators.back().string]);
            std::vector<expression::ExprShPtr> argumentExpressions{expressions.end() - arity, expressions.end()};
            expressions.resize(expressions.size() - arity);
            operatorExpression->setExpressions(std::move(argumentExpressions));
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
            auto rightExpr = expressions.back();
            expressions.pop_back();
            auto leftExpr = expressions.back();
            expressions.pop_back();
            if (!assignOperatorFunction_[operators.back().string])
            {
                throw std::invalid_argument("can not find assignment operator " + operators.back().string);
            }
            auto assignExpr = std::make_shared<expression::AssignExpression>(leftExpr,
                                                                             rightExpr,
                                                                             assignOperatorFunction_[operators.back().string]);
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
            case token::tokenCategory::OPERATOR_BI:
                popArOperator(token::getTokenOperatorArity(operators.back().category));
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
                while (!operators.empty() &&
                       getOperatorPriority(operators.back().string) > getOperatorPriority(op.string))
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
                expressions.push_back(std::make_shared<expression::VariableAccess>(token->getTokenString()));
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

        if (operatorPriority_.find(op) == operatorPriority_.end())
        {
            throw std::invalid_argument("undefined operator " + op);
        }
        return operatorPriority_[op];
    }

    std::vector<token::tokenCategory_t> singleIndentationBlock_;
    std::stack<UnfinishedExpression> unfinishedExpressions_;
    std::unordered_map<std::string, int> operatorPriority_;
    std::unordered_map<std::string, expression::FunctionCall::FunctionType> operatorFunction_;
    std::unordered_map<std::string, expression::AssignExpression::FunctionType> assignOperatorFunction_;
}; // namespace interpreter
} // namespace interpreter
} // namespace peach
