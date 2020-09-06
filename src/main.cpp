#include <algorithm>
#include <fstream>
#include <iostream>
#include <memory>
#include <numeric>
#include <streambuf>
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
                {"&", tokenCategory::OPERATOR_BI},
                {"|", tokenCategory::OPERATOR_BI},
                {"*", tokenCategory::OPERATOR_BI},
                {"/", tokenCategory::OPERATOR_BI},
                {"%", tokenCategory::OPERATOR_BI},
                {"+", tokenCategory::OPERATOR_BI},
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

    auto inputFile = std::ifstream("simple.pch");
    std::string programText((std::istreambuf_iterator<char>(inputFile)),
                            std::istreambuf_iterator<char>());

    // std::cout << programText << std::endl;

    auto tokens = finder.tokenizeText(programText, {
                                                       {"if", tokenCategory::COND_IF},
                                                       {"else", tokenCategory::COND_ELSE},
                                                       {"while", tokenCategory::LOOP_WHILE},
                                                   });

    for (const auto &tk : tokens)
    {
        std::cout << "'" << tk->getTokenString() << "'" << ' ' << tk->getLine() << ' ' << tk->getPosition() << ' ';
        peach::token::printCategory(tk->getCategory());
        std::cout << std::endl;
    }

    auto interpreter = peach::interpreter::Interpreter(std::vector<tokenCategory_t>{
                                                           tokenCategory::SEP_SPACE,
                                                           tokenCategory::SEP_SPACE,
                                                           tokenCategory::SEP_SPACE,
                                                           tokenCategory::SEP_SPACE,
                                                       },
                                                       std::vector<std::variant<peach::interpreter::UnaryOperatorInfo, peach::interpreter::BinaryOperatorInfo, peach::interpreter::AssignationInfo>>{
                                                           peach::interpreter::UnaryOperatorInfo{
                                                               [](std::tuple<peach::expression::VType> args) {
                                                                   return !std::get<0>(args);
                                                               },
                                                               "!",
                                                               tokenCategory::OPERATOR_UN,
                                                           },
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
                                                                   return std::get<0>(args) % std::get<1>(args);
                                                               },
                                                               "%",
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
                                                           peach::interpreter::BinaryOperatorInfo{
                                                               [](std::tuple<peach::expression::VType, peach::expression::VType> args) {
                                                                   return std::get<0>(args) == std::get<1>(args);
                                                               },
                                                               "==",
                                                               tokenCategory::OPERATOR_BI,
                                                           },
                                                           peach::interpreter::BinaryOperatorInfo{
                                                               [](std::tuple<peach::expression::VType, peach::expression::VType> args) {
                                                                   return std::get<0>(args) != std::get<1>(args);
                                                               },
                                                               "!=",
                                                               tokenCategory::OPERATOR_BI,
                                                           },
                                                           peach::interpreter::BinaryOperatorInfo{
                                                               [](std::tuple<peach::expression::VType, peach::expression::VType> args) {
                                                                   return std::get<0>(args) > std::get<1>(args);
                                                               },
                                                               ">",
                                                               tokenCategory::OPERATOR_BI,
                                                           },
                                                           peach::interpreter::BinaryOperatorInfo{
                                                               [](std::tuple<peach::expression::VType, peach::expression::VType> args) {
                                                                   return std::get<0>(args) >= std::get<1>(args);
                                                               },
                                                               ">=",
                                                               tokenCategory::OPERATOR_BI,
                                                           },
                                                           peach::interpreter::BinaryOperatorInfo{
                                                               [](std::tuple<peach::expression::VType, peach::expression::VType> args) {
                                                                   return std::get<0>(args) < std::get<1>(args);
                                                               },
                                                               "<",
                                                               tokenCategory::OPERATOR_BI,
                                                           },
                                                           peach::interpreter::BinaryOperatorInfo{
                                                               [](std::tuple<peach::expression::VType, peach::expression::VType> args) {
                                                                   return std::get<0>(args) <= std::get<1>(args);
                                                               },
                                                               "<=",
                                                               tokenCategory::OPERATOR_BI,
                                                           },
                                                           peach::interpreter::BinaryOperatorInfo{
                                                               [](std::tuple<peach::expression::VType, peach::expression::VType> args) {
                                                                   return std::get<0>(args) || std::get<1>(args);
                                                               },
                                                               "|",
                                                               tokenCategory::OPERATOR_BI,
                                                           },
                                                           peach::interpreter::BinaryOperatorInfo{
                                                               [](std::tuple<peach::expression::VType, peach::expression::VType> args) {
                                                                   return std::get<0>(args) && std::get<1>(args);
                                                               },
                                                               "&",
                                                               tokenCategory::OPERATOR_BI,
                                                           },
                                                           peach::interpreter::AssignationInfo{":="},
                                                       });

    interpreter.interpretateLines(tokens.begin(), tokens.end());
    auto program = interpreter.getInterpretationResult();
    peach::expression::Scope scope;
    scope["var"] = 100;
    std::cout << program->eval(scope) << std::endl;

    return 0;
}
