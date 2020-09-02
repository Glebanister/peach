#include <algorithm>
#include <iostream>
#include <memory>
#include <numeric>
#include <unordered_map>
#include <vector>

#include "Expression.hpp"
#include "FsmCollection.hpp"
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
    // auto finder = peach::fsm::FsmCollection();
    // finder
    //     .buildAppendFsm<peach::fsm::NameFinder>()                                     //
    //     .buildAppendFsm<peach::fsm::NumberFinder>(tokenCategory::VALUE_FLOATING, '.') //
    //     .buildAppendFsm<peach::fsm::NumberFinder>(tokenCategory::VALUE_INT)           //
    //     .buildAppendFsm<peach::fsm::OperatorFinder>(
    //         std::vector<std::pair<std::string, tokenCategory_t>>{
    //             {"!", tokenCategory::OPERATOR_UN},
    //             {"&", tokenCategory::OPERATOR_BI},
    //             {"|", tokenCategory::OPERATOR_BI},
    //             {"*", tokenCategory::OPERATOR_BI},
    //             {"/", tokenCategory::OPERATOR_BI},
    //             {"%", tokenCategory::OPERATOR_BI},
    //             {"+", tokenCategory::OPERATOR_BI},
    //             {"-", tokenCategory::OPERATOR_BI},
    //             {"=", tokenCategory::ASSIGNMENT},
    //             {"==", tokenCategory::OPERATOR_BI},
    //             {"&=", tokenCategory::ASSIGNMENT},
    //             {"|=", tokenCategory::ASSIGNMENT},
    //             {"*=", tokenCategory::ASSIGNMENT},
    //             {"/=", tokenCategory::ASSIGNMENT},
    //             {"%=", tokenCategory::ASSIGNMENT},
    //             {"+=", tokenCategory::ASSIGNMENT},
    //             {"-=", tokenCategory::ASSIGNMENT},
    //         }) //
    //     .buildAppendFsm<peach::fsm::SingleCharFinder>(
    //         std::vector<std::pair<char, tokenCategory_t>>{
    //             {'\n', tokenCategory::SEP_ENDL},
    //             {' ', tokenCategory::SEP_SPACE},
    //             {'\t', tokenCategory::SEP_TAB},
    //             {'(', tokenCategory::BRACKET_OPEN},
    //             {')', tokenCategory::BRACKET_CLOSE},
    //             {':', tokenCategory::COLON},
    //             {';', tokenCategory::SEMICOLON},
    //         }) //
    //     ;

    // auto text = "whi = 1 -4\n"
    //             "whilel=4123123\n"
    //             "i = 0\n"
    //             "while (res != 0)\n"
    //             "    i = i + 1\n"
    //             "    res = res - 1\n"
    //             "    if i % 2 == 0:\n"
    //             "        res = res + a + b + i\n"
    //             "    elif i % 2 == 1:\n"
    //             "        res_2 = 0\n"
    //             "    else:\n"
    //             "        res = res + c - i\n"
    //             "res += c\n";

    // for (const auto &token : finder.tokenizeText(text, {
    //                                                        {"if", tokenCategory::COND_IF},
    //                                                        {"elif", tokenCategory::COND_ELIF},
    //                                                        {"else", tokenCategory::COND_ELSE},
    //                                                        {"while", tokenCategory::LOOP_WHILE},
    //                                                    }))
    // {
    //     if (token->getCategory() != peach::token::tokenCategory::UNDEFINED)
    //     {
    //         std::cout << token->getTokenString();
    //         // std::string str = token->getTokenString();
    //         // if (str == "\n")
    //         // {
    //         //     str = "\\n";
    //         // }
    //         // std::cout << token->getPosition() << ' ' << '\'' << str << '\'' << ' ';
    //         // peach::token::printCategory(token->getCategory());
    //         // std::cout << std::endl;
    //     }
    // }

    using namespace peach;

    expression::Scope emptyScope;
    expression::FunctionCall<1> neg({std::make_unique<expression::VTypeValue>(10)},
                                    [](std::tuple<peach::expression::VType> args) { return -std::get<0>(args); });
    std::cout << neg.eval(emptyScope).value << std::endl;

    return 0;
}
