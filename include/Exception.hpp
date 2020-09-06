#pragma once

#include <stdexcept>

#include "Token.hpp"

namespace peach
{
namespace exception
{
class PeachException : public std::exception
{
public:
    PeachException(const std::string &exceptionClass,
                   const std::string &exceptionMessage = "")
        : report_(exceptionClass + (exceptionClass.empty() ? "" : (": " + exceptionMessage)))
    {
    }

    const char *what() const throw()
    {
        return report_.c_str();
    }

private:
    const std::string report_;
};

class PositionalError : public PeachException
{
public:
    PositionalError(std::size_t line,
                    std::size_t position,
                    const std::string &exceptionClass,
                    const std::string &comment)
        : PeachException(exceptionClass,
                         comment + " at " +
                             std::to_string(line) +
                             ":" + std::to_string(position)),
          line_(line),
          position_(position)
    {
    }

    std::size_t getLine() const noexcept { return line_; }
    std::size_t getPosition() const noexcept { return position_; }

private:
    const std::size_t line_, position_;
};

template <typename ExceptionT,
          typename = decltype(std::declval<ExceptionT>().getLine()),
          typename = decltype(std::declval<ExceptionT>().getPosition())>
inline void throwFromTokenIterator(const std::vector<token::TokenPtr>::iterator &it)
{
    throw ExceptionT((*it)->getLine(),
                     (*it)->getPosition());
}

class IndentationError : public PositionalError
{
public:
    IndentationError(std::size_t line,
                     std::size_t position)
        : PositionalError(line,
                          position,
                          "IndentationError", "bad indentation")
    {
    }
};

class SyntaxError : public PositionalError
{
public:
    SyntaxError(std::size_t line,
                std::size_t position)
        : PositionalError(line,
                          position,
                          "SyntaxError", "invalid syntax")
    {
    }
};

class UndefinedTokenError : public PositionalError
{
public:
    UndefinedTokenError(std::size_t line,
                        std::size_t position)
        : PositionalError(line,
                          position,
                          "UndefinedTokenError", "can not recognize token")
    {
    }
};

class UnexpectedTokenError : public PositionalError
{
public:
    UnexpectedTokenError(std::size_t line,
                         std::size_t position)
        : PositionalError(line,
                          position,
                          "UnexpectedTokenError", "token is not expected")
    {
    }
};

class UnexpectedElseError : public PositionalError
{
public:
    UnexpectedElseError(std::size_t line,
                        std::size_t position)
        : PositionalError(line,
                          position,
                          "UnexpectedElseError", "can not process 'else' if it not preceded by 'if'")
    {
    }
};

class UndefinedOperatorError : public PositionalError
{
public:
    UndefinedOperatorError(std::size_t line,
                           std::size_t position)
        : PositionalError(line,
                          position,
                          "UndefinedOperatorError", "can't find operator")
    {
    }
};

class BracketDisbalanceError : public PositionalError
{
public:
    BracketDisbalanceError(std::size_t line,
                           std::size_t position)
        : PositionalError(line,
                          position,
                          "BracketDisbalanceError", "can't match bracket")
    {
    }
};

class InterruptionError : public PeachException
{
public:
    InterruptionError()
        : PeachException("InterruptionError", "interpretation unexpectedly finished")
    {
    }
};
} // namespace exception
} // namespace peach
