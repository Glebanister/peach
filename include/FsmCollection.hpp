#pragma once

#include <vector>

#include "FiniteStateMachine.hpp"

namespace peach
{
namespace fsm
{
// Collection of FiniteStateMachines
// First attach FSMs, then tokenize text
// Uses FSMs in order, they were attached to collection
class FsmCollection
{
public:
    // Appends new fsm to collection
    // Returns reference on this collection (so you can perform "collection.appendFsm(std::move(...)).appendFsm(...).appendFsm(...)...")
    // Be careful: function takes object only with rvalue reference. If you build some fsm in code, you can move it to this function.
    FsmCollection &appendFsm(std::unique_ptr<FiniteStateMachine> &&machine)
    {
        collection_.emplace_back(std::move(machine));
        currentFsmId_ = 0;
        return *this;
    }

    // Builds and appends new fsm to collection
    // Returns reference on this collection (so you can perform "collection.appendFsm(std::move(...)).appendFsm(...).appendFsm(...)...")
    // Be careful: function takes object only with rvalue reference. If you build some fsm in code, you can move it to this function.
    template <typename Fsm, typename... FsmArgs>
    FsmCollection &buildAppendFsm(FsmArgs &&... args)
    {
        return appendFsm(std::make_unique<Fsm>(std::forward<FsmArgs>(args)...));
    }

    // Tokenizes given text into fsm tokens. Changes reservedKeyword categories to given ones.
    // Returns vector of tokens
    std::vector<std::unique_ptr<token::Token>> tokenizeText(const std::string &text,
                                                            const std::vector<std::pair<std::string, token::tokenCategory_t>> &reservedKeywords)
    {
        std::vector<std::unique_ptr<token::Token>> tokens;
        auto processChar = [&](char c) {
            auto newToken = pushNextChar(c);
            if (newToken)
            {
                tokens.emplace_back(std::move(newToken));
            }
        };
        std::for_each(text.begin(), text.end(), processChar);
        processChar('\0');
        for (const auto &token : tokens)
        {
            for (const auto &[keyword, category] : reservedKeywords)
            {
                if (token->getTokenString() == keyword)
                {
                    token->setCategory(category);
                }
            }
        }
        return tokens;
    }

private:
    // Increases number of current FSM, drops to 0, if there is no more FSMs in collection.
    // Returns if number has been increased.
    bool setNextFsm() noexcept
    {
        if (++currentFsmId_ == static_cast<int>(collection_.size()))
        {
            currentFsmId_ = 0;
            return false;
        }
        else
        {
            return true;
        }
    }

    void resetFsmId() noexcept
    {
        currentFsmId_ = 0;
    }

    std::unique_ptr<FiniteStateMachine> &getCurrentFsm()
    {
        if (currentFsmId_ < 0)
        {
            throw std::out_of_range("collection must have at least one FSM to apply this operation");
        }
        return collection_.at(currentFsmId_);
    }

    // Pushes next char in text to FSMs collection.
    // Returns unique pointer to new token, if terminal has been reached, nullptr otherwise.
    std::unique_ptr<token::Token> pushNextChar(char c)
    {

        if (auto [successPush, prevNodeCategory] = pushCharRecursively(c); !successPush)
        {
            currentToken_ += c;
            return buildAndMoveCurrentToken(prevNodeCategory);
        }
        else if (prevNodeCategory != token::tokenCategory::UNDEFINED)
        {
            auto result = buildAndMoveCurrentToken(prevNodeCategory);
            if (auto unexpectedToken = pushNextChar(c); unexpectedToken &&
                                                        unexpectedToken->getCategory() != token::tokenCategory::UNDEFINED)
            {
                throw std::logic_error("distance between FSM root and any node should be at least 2 transitions"); // TODO: bad description
            }
            return result;
        }
        currentToken_ += c;
        return nullptr;
    }

    // Builds new token from currentToken_ currentTokenBeginPos_ and current node category
    // Returns unique pointer on new token
    std::unique_ptr<token::Token> buildAndMoveCurrentToken(token::tokenCategory_t category)
    {
        auto tokenBeginPos = currentTokenBeginPos_;
        currentTokenBeginPos_ += currentToken_.length();
        auto result = std::make_unique<token::Token>(category,
                                                     std::move(currentToken_),
                                                     tokenBeginPos);
        currentToken_.clear();
        resetFsmId();
        return result;
    }

    // Tries to fit current token in fsm collection in order of adding to collection
    // Returns pair [if operation was successfull; previous node category]
    std::pair<bool, token::tokenCategory_t> pushCharRecursively(char c)
    {
        if (auto [successfullPush, prevNodeCategory] = getCurrentFsm()->pushChar(c); !successfullPush)
        {
            bool success = false;
            while (!success)
            {
                if (!setNextFsm())
                {
                    return {false, prevNodeCategory};
                }
                success = true;
                for (char tokenC : currentToken_ + c)
                {
                    auto [curPushResult, curPrevNodeCategory] = getCurrentFsm()->pushChar(tokenC);
                    if (!curPushResult)
                    {
                        success = false;
                        break;
                    }
                    else
                    {
                        prevNodeCategory = curPrevNodeCategory;
                    }
                }
            }
            return {true, prevNodeCategory};
        }
        else
        {
            return {true, prevNodeCategory};
        }
    }

    std::vector<std::unique_ptr<FiniteStateMachine>> collection_; // FSM collection
    int currentFsmId_ = -1;                                       // current fsm id in vector
    std::string currentToken_ = "";                               // current token string
    std::size_t currentTokenBeginPos_ = 0;                        // current token begin position
};
} // namespace fsm
} // namespace peach
