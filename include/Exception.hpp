#pragma once

#include <stdexcept>

namespace peach
{
namespace exception
{
class PeachException : public std::exception
{
public:
    PeachException(const std::string &exceptionClass,
                   const std::string &exceptionMessage = "")
        : class_(exceptionClass),
          message_(exceptionMessage)
    {
    }

    std::string getReport() const
    {
        auto result = class_;
        if (!message_.empty())
        {
            result += ": " + message_;
        }
        return result;
    }

    const char *what() const throw()
    {
        return getReport().c_str();
    }

private:
    const std::string class_, message_;
};

class PositionalException : PeachException
{
public:
    PositionalException(const std::string &filename,
                        std::size_t line,
                        std::size_t position,
                        const std::string &exceptionClass,
                        const std::string &comment)
        : PeachException(exceptionClass,
                         comment + " in " + filename +
                             ":" + std::to_string(line) +
                             ":" + std::to_string(position)),
          line_(line),
          position_(position),
          filename_(filename)
    {
    }

    std::size_t getLine() const noexcept { return line_; }
    std::size_t getPosition() const noexcept { return position_; }
    std::string getFilename() const { return filename_; }

private:
    const std::size_t line_, position_;
    const std::string filename_;
};

class IndentationException : public PositionalException
{
public:
    IndentationException(const std::string &filename,
                         std::size_t line,
                         std::size_t position)
        : PositionalException(filename,
                              line,
                              position,
                              "IndentationException", "bad indentation")
    {
    }
};
} // namespace exception
} // namespace peach
