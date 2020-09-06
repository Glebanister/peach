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
        DECLARATION,
    };

    struct UnfinishedExpression
    {
        expression::ExprShPtr expression;
        std::shared_ptr<expression::ExpressionSequence> sequence;
        indentationBlockType type;
    };

public:
    Interpreter(std::vector<token::tokenCategory_t> singleIndentationBlock,
                std::vector<OperatorInfo> operatorsOrder,
                std::string assignationPattern = "=")
        : singleIndentationBlock_(std::move(singleIndentationBlock)),
          operators_(std::move(operatorsOrder))
    {
        reset();
        std::size_t curPrior = operators_.size() + 1;
        for (const auto &op : operators_)
        {
            operatorPriority_[op.tokenString] = curPrior--;
            operatorFunction_[op.tokenString] = op.functor;
        }
        operatorPriority_[assignationPattern] = 0;
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
        else if (lineCategory == indentationBlockType::DECLARATION)
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
            {token::tokenCategory::DECLARATION, indentationBlockType::DECLARATION},
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
            auto assignExpr = std::make_shared<expression::AssignExpression>(leftExpr, rightExpr);
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
    std::vector<OperatorInfo> operators_;
    std::unordered_map<std::string, int> operatorPriority_;
    std::unordered_map<std::string, expression::FunctionCall::FunctionType> operatorFunction_;
}; // namespace interpreter
} // namespace interpreter
} // namespace peach
