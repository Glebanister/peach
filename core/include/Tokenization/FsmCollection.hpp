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
        if (text.empty())
        {
            return {};
        }
        std::vector<std::unique_ptr<token::Token>> tokens;
        auto processChar = [&](char c) {
            pushNextChar(c, tokens);
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

    // Resets information about inputted text
    void reset()
    {
        resetFsmId();
        currentToken_ = "";
        currentTokenLineBeginPos_ = 0;
        currentTokenTextBeginPos_ = 0;
        currentTokenLine_ = 0;
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
    // Adds unique pointer to tokens, if terminal has been reached.
    void pushNextChar(char c, std::vector<std::unique_ptr<token::Token>> &tokens)
    {
        auto addToken = [&](std::unique_ptr<token::Token> &&tk) {
            if (!tk->getTokenString().empty() && tk->getTokenString()[0] != '\0')
            {
                tokens.emplace_back(std::move(tk));
            }
        };

        if (auto [successPush, prevNodeCategory] = pushCharRecursively(c); !successPush)
        {
            currentToken_ += c;
            if (auto tk = buildAndMoveCurrentToken(prevNodeCategory))
            {
                if (!tk->getTokenString().empty() && tk->getTokenString()[0] != '\0')
                {
                    addToken(std::move(tk));
                }
            }
            return;
        }
        else if (prevNodeCategory != token::tokenCategory::UNDEFINED)
        {
            addToken(buildAndMoveCurrentToken(prevNodeCategory));
            pushNextChar(c, tokens);
            return;
        }
        currentToken_ += c;
    }

    // Builds new token from currentToken_ currentTokenBeginPos_ and current node category
    // Returns unique pointer on new token
    std::unique_ptr<token::Token> buildAndMoveCurrentToken(token::tokenCategory_t category)
    {
        auto tokenLineBeginPos = currentTokenLineBeginPos_;
        auto tokenTextBeginPos = currentTokenTextBeginPos_;
        currentTokenTextBeginPos_ += currentToken_.length();
        currentTokenLineBeginPos_ += currentToken_.length();
        if (!currentToken_.empty() && token::isEndline(currentToken_[0]))
        {
            currentTokenLineBeginPos_ = 0;
            ++currentTokenLine_;
        }
        auto result = std::make_unique<token::Token>(category,
                                                     std::move(currentToken_),
                                                     currentTokenLine_,
                                                     tokenLineBeginPos,
                                                     tokenTextBeginPos);
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
    std::size_t currentTokenLineBeginPos_ = 0;                    // current token begin position in line
    std::size_t currentTokenTextBeginPos_ = 0;                    // current token begin position in text
    std::size_t currentTokenLine_ = 0;                            // current token line number
};
} // namespace fsm
} // namespace peach
