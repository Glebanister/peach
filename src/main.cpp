#include <algorithm>
#include <iostream>
#include <memory>
#include <numeric>
#include <unordered_map>
#include <vector>

#include "Expression.hpp"
#include "FsmCollection.hpp"
#include "Interpreter.hpp"
#include "NameFinder.hpp"
#include "NumberFinder.hpp"
#include "OperatorFinder.hpp"
#include "SingleCharFinder.hpp"
#include "Transition.hpp"

int main()
{
    using peach::token::tokenCategory;
    using peach::token::tokenCategory_t;
    using namespace peach::transition;
    auto finder = peach::fsm::FsmCollection();
    finder
        .buildAppendFsm<peach::fsm::NameFinder>()                                     //
        .buildAppendFsm<peach::fsm::NumberFinder>(tokenCategory::VALUE_FLOATING, '.') //
        .buildAppendFsm<peach::fsm::NumberFinder>(tokenCategory::VALUE_INT)           //
        .buildAppendFsm<peach::fsm::OperatorFinder>(
            std::vector<std::pair<std::string, tokenCategory_t>>{
                {"!", tokenCategory::OPERATOR_UN},
                {"&", tokenCategory::OPERATOR_BI},
                {"|", tokenCategory::OPERATOR_BI},
                {"*", tokenCategory::OPERATOR_BI},
                {"/", tokenCategory::OPERATOR_BI},
                {"%", tokenCategory::OPERATOR_BI},
                {"+", tokenCategory::OPERATOR_BI},
                {"-", tokenCategory::OPERATOR_BI},
                {"=", tokenCategory::ASSIGNMENT},
                {"==", tokenCategory::OPERATOR_BI},
                {"&=", tokenCategory::ASSIGNMENT},
                {"|=", tokenCategory::ASSIGNMENT},
                {"*=", tokenCategory::ASSIGNMENT},
                {"/=", tokenCategory::ASSIGNMENT},
                {"%=", tokenCategory::ASSIGNMENT},
                {"+=", tokenCategory::ASSIGNMENT},
                {"-=", tokenCategory::ASSIGNMENT},
            }) //
        .buildAppendFsm<peach::fsm::SingleCharFinder>(
            std::vector<std::pair<char, tokenCategory_t>>{
                {'\n', tokenCategory::SEP_ENDL},
                {' ', tokenCategory::SEP_SPACE},
                {'\t', tokenCategory::SEP_TAB},
                {'(', tokenCategory::BRACKET_OPEN},
                {')', tokenCategory::BRACKET_CLOSE},
                {':', tokenCategory::COLON},
                {';', tokenCategory::SEMICOLON},
            }) //
        ;

    auto text = "a = 2 - 3\n";

    // "while (a != 0)\n"
    // "    if (a == 3)\n"
    // "        a = a - 2\n"
    // "    else\n"
    // "        a = a - 1\n"
    // "a = 10\n";

    auto tokens = finder.tokenizeText(text, {
                                                {"if", tokenCategory::COND_IF},
                                                {"else", tokenCategory::COND_ELSE},
                                                {"while", tokenCategory::LOOP_WHILE},
                                            });

    for (auto &token : tokens)
    {
        std::cout << "'" << token->getTokenString() << "'" << std::endl;
    }

    auto interpreter = peach::interpreter::Interpreter(std::vector<tokenCategory_t>{
                                                           tokenCategory::SEP_SPACE,
                                                           tokenCategory::SEP_SPACE,
                                                           tokenCategory::SEP_SPACE,
                                                           tokenCategory::SEP_SPACE,
                                                       },
                                                       std::vector<std::variant<peach::interpreter::UnaryOperatorInfo, peach::interpreter::BinaryOperatorInfo, peach::interpreter::AssignationInfo>>{
                                                           peach::interpreter::BinaryOperatorInfo{
                                                               [](std::tuple<peach::expression::VType, peach::expression::VType> args) {
                                                                   return std::get<0>(args) * std::get<1>(args);
                                                               },
                                                               "*",
                                                               tokenCategory::OPERATOR_BI,
                                                           },
                                                           peach::interpreter::BinaryOperatorInfo{
                                                               [](std::tuple<peach::expression::VType, peach::expression::VType> args) {
                                                                   return std::get<0>(args) / std::get<1>(args);
                                                               },
                                                               "/",
                                                               tokenCategory::OPERATOR_BI,
                                                           },
                                                           peach::interpreter::BinaryOperatorInfo{
                                                               [](std::tuple<peach::expression::VType, peach::expression::VType> args) {
                                                                   return std::get<0>(args) + std::get<1>(args);
                                                               },
                                                               "+",
                                                               tokenCategory::OPERATOR_BI,
                                                           },
                                                           peach::interpreter::BinaryOperatorInfo{
                                                               [](std::tuple<peach::expression::VType, peach::expression::VType> args) {
                                                                   return std::get<0>(args) - std::get<1>(args);
                                                               },
                                                               "-",
                                                               tokenCategory::OPERATOR_BI,
                                                           },
                                                           peach::interpreter::AssignationInfo{"="},
                                                       });

    interpreter.interpretateLine(tokens.begin(), tokens.end());
    auto program = std::move(interpreter).getInterpretationResult();
    peach::expression::Scope scope;
    std::cout << program->eval(scope) << std::endl;
    std::cout << scope["a"] << std::endl;

    return 0;
}
