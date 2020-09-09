#pragma once

#include <vector>

#include "Finders/NameFinder.hpp"
#include "Finders/NumberFinder.hpp"
#include "Finders/OperatorFinder.hpp"
#include "Finders/SingleCharFinder.hpp"

#include "FsmCollection.hpp"
#include "Interpreter.hpp"

namespace peach
{
namespace cli
{
// Command line interface for peach
class PeachCli
{
public:
    PeachCli()
        : interpreter_(std::vector<token::tokenCategory_t>{
                           token::tokenCategory::SEP_TAB,
                       },
                       {
                           {
                               [](peach::expression::PeachTuple args) {
                                   return !args[0];
                               },
                               "!",
                               token::tokenCategory::OPERATOR_UN,
                           },
                           {
                               [](peach::expression::PeachTuple args) {
                                   expression::VType res = 1;
                                   expression::VType deg = args[1];
                                   expression::VType x = args[0];
                                   while (deg)
                                   {
                                       if (!(deg & 1))
                                       {
                                           x *= x;
                                           deg >>= 1;
                                       }
                                       else
                                       {
                                           res *= x;
                                           --deg;
                                       }
                                   }
                                   return res;
                               },
                               "**",
                               token::tokenCategory::OPERATOR_BI,
                           },
                           {
                               [](peach::expression::PeachTuple args) {
                                   return args[0] * args[1];
                               },
                               "*",
                               token::tokenCategory::OPERATOR_BI,
                           },
                           {
                               [](peach::expression::PeachTuple args) {
                                   if (args[1] == 0)
                                   {
                                       throw exception::ZeroDivisionError();
                                   }
                                   return args[0] / args[1];
                               },
                               "/",
                               token::tokenCategory::OPERATOR_BI,
                           },
                           {
                               [](peach::expression::PeachTuple args) {
                                   return args[0] % args[1];
                               },
                               "%",
                               token::tokenCategory::OPERATOR_BI,
                           },
                           {
                               [](peach::expression::PeachTuple args) {
                                   return args[0] + args[1];
                               },
                               "+",
                               token::tokenCategory::OPERATOR_BI,
                           },
                           {
                               [](peach::expression::PeachTuple args) {
                                   return args[0] - args[1];
                               },
                               "-",
                               token::tokenCategory::OPERATOR_BI,
                           },
                           {
                               [](peach::expression::PeachTuple args) {
                                   return args[0] == args[1];
                               },
                               "==",
                               token::tokenCategory::OPERATOR_BI,
                           },
                           {
                               [](peach::expression::PeachTuple args) {
                                   return args[0] != args[1];
                               },
                               "!=",
                               token::tokenCategory::OPERATOR_BI,
                           },
                           {
                               [](peach::expression::PeachTuple args) {
                                   return args[0] > args[1];
                               },
                               ">",
                               token::tokenCategory::OPERATOR_BI,
                           },
                           {
                               [](peach::expression::PeachTuple args) {
                                   return args[0] >= args[1];
                               },
                               ">=",
                               token::tokenCategory::OPERATOR_BI,
                           },
                           {
                               [](peach::expression::PeachTuple args) {
                                   return args[0] < args[1];
                               },
                               "<",
                               token::tokenCategory::OPERATOR_BI,
                           },
                           {
                               [](peach::expression::PeachTuple args) {
                                   return args[0] <= args[1];
                               },
                               "<=",
                               token::tokenCategory::OPERATOR_BI,
                           },
                           {
                               [](peach::expression::PeachTuple args) {
                                   return args[0] || args[1];
                               },
                               "|",
                               token::tokenCategory::OPERATOR_BI,
                           },
                           {
                               [](peach::expression::PeachTuple args) {
                                   return args[0] && args[1];
                               },
                               "&",
                               token::tokenCategory::OPERATOR_BI,
                           },
                       },
                       {
                           {
                               [](peach::expression::VType &left, peach::expression::VType right) {
                                   left = right;
                               },
                               "=",
                               token::tokenCategory::ASSIGNMENT,
                           },
                           {
                               [](peach::expression::VType &left, peach::expression::VType right) {
                                   left += right;
                               },
                               "+=",
                               token::tokenCategory::ASSIGNMENT,
                           },
                           {
                               [](peach::expression::VType &left, peach::expression::VType right) {
                                   left -= right;
                               },
                               "-=",
                               token::tokenCategory::ASSIGNMENT,
                           },
                           {
                               [](peach::expression::VType &left, peach::expression::VType right) {
                                   left *= right;
                               },
                               "*=",
                               token::tokenCategory::ASSIGNMENT,
                           },
                           {
                               [](peach::expression::VType &left, peach::expression::VType right) {
                                   left /= right;
                               },
                               "/=",
                               token::tokenCategory::ASSIGNMENT,
                           },
                           {
                               [](peach::expression::VType &left, peach::expression::VType right) {
                                   left %= right;
                               },
                               "%=",
                               token::tokenCategory::ASSIGNMENT,
                           },
                           {
                               [](peach::expression::VType &left, peach::expression::VType right) {
                                   left &= right;
                               },
                               "&=",
                               token::tokenCategory::ASSIGNMENT,
                           },
                           {
                               [](peach::expression::VType &left, peach::expression::VType right) {
                                   left |= right;
                               },
                               "|=",
                               token::tokenCategory::ASSIGNMENT,
                           },
                       }),
          keywords_({
              {"if", token::tokenCategory::COND_IF},
              {"else", token::tokenCategory::COND_ELSE},
              {"while", token::tokenCategory::LOOP_WHILE},
              {"let", token::tokenCategory::DECLARATION},
          })

    {
        using peach::token::tokenCategory;
        using peach::token::tokenCategory_t;
        using namespace peach::transition;
        tokenizator_
            .buildAppendFsm<peach::fsm::NameFinder>()                                     //
            .buildAppendFsm<peach::fsm::NumberFinder>(tokenCategory::VALUE_FLOATING, '.') //
            .buildAppendFsm<peach::fsm::NumberFinder>(tokenCategory::VALUE_INT)           //
            .buildAppendFsm<peach::fsm::OperatorFinder>(
                std::vector<std::pair<std::string, tokenCategory_t>>{
                    {"&=", tokenCategory::ASSIGNMENT},
                    {"&", tokenCategory::OPERATOR_BI},
                    {"|=", tokenCategory::ASSIGNMENT},
                    {"|", tokenCategory::OPERATOR_BI},
                    {"*=", tokenCategory::ASSIGNMENT},
                    {"**", tokenCategory::OPERATOR_BI},
                    {"*", tokenCategory::OPERATOR_BI},
                    {"/=", tokenCategory::ASSIGNMENT},
                    {"/", tokenCategory::OPERATOR_BI},
                    {"%=", tokenCategory::ASSIGNMENT},
                    {"%", tokenCategory::OPERATOR_BI},
                    {"+=", tokenCategory::ASSIGNMENT},
                    {"+", tokenCategory::OPERATOR_BI},
                    {"-=", tokenCategory::ASSIGNMENT},
                    {"-", tokenCategory::OPERATOR_BI},
                    {"==", tokenCategory::OPERATOR_BI},
                    {"=", tokenCategory::ASSIGNMENT},
                    {"!=", tokenCategory::OPERATOR_BI},
                    {"!", tokenCategory::OPERATOR_UN},
                    {">", tokenCategory::OPERATOR_BI},
                    {"<", tokenCategory::OPERATOR_BI},
                    {">=", tokenCategory::OPERATOR_BI},
                    {"<=", tokenCategory::OPERATOR_BI},
                }) //
            .buildAppendFsm<peach::fsm::SingleCharFinder>(
                std::vector<std::pair<char, tokenCategory_t>>{
                    {'\n', tokenCategory::SEP_ENDL},
                    {' ', tokenCategory::SEP_SPACE},
                    {'\t', tokenCategory::SEP_TAB},
                    {'(', tokenCategory::BRACKET_OPEN},
                    {')', tokenCategory::BRACKET_CLOSE},
                }) //
            ;
    }

    // Executes program from istream
    void executeProgram(std::ifstream &ifs, std::ostream &os)
    {
        std::string programText((std::istreambuf_iterator<char>(ifs)),
                                std::istreambuf_iterator<char>());
        auto tokens = tokenizator_.tokenizeText(programText, keywords_);
        try
        {
            interpreter_.interpretateLines(tokens.begin(), tokens.end());
            os << interpreter_.getInterpretationResult()->eval(scope_) << std::endl;
        }
        catch (const std::exception &e)
        {
            os << e.what() << '\n';
        }
    }

    // Main interface loop, activates cli
    void loop(std::istream &is, std::ostream &os)
    {
        os << "Peach" << std::endl;
        while (!is.eof())
        {
            if (interpreter_.getIndentationLevel() == 0)
            {
                os << ">>> ";
            }
            else
            {
                os << "... ";
            }
            if (interpreter_.getIndentationLevel() == 0)
            {
                interpreter_.reset();
            }
            tokenizator_.reset();
            std::string input;
            std::getline(is, input);
            auto tokens = tokenizator_.tokenizeText(input, keywords_);
            try
            {
                interpreter_.interpretateLine(tokens.begin(), tokens.end());
                if (interpreter_.getIndentationLevel() == 0 || tokens.empty())
                {
                    os << interpreter_.getInterpretationResult()->eval(scope_) << std::endl;
                }
            }
            catch (const std::exception &e)
            {
                os << e.what() << '\n';
            }
        }
    }

    // Retuns reference on current state of cli
    expression::Scope &getScope()
    {
        return scope_;
    }

private:
    fsm::FsmCollection tokenizator_;
    interpreter::Interpreter interpreter_;
    const std::vector<std::pair<std::string, peach::token::tokenCategory_t>> keywords_;
    expression::Scope scope_;
};
} // namespace cli
} // namespace peach
