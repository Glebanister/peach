#include <algorithm>
#include <iostream>
#include <memory>
#include <numeric>
#include <unordered_map>
#include <vector>

#include "FsmCollection.hpp"
#include "NameFinder.hpp"
#include "OperatorFinder.hpp"
#include "SingleCharFinder.hpp"

#include "Transition.hpp"

int main()
{
    using peach::token::tokenCategory;
    using namespace peach::transition;
    auto finder = peach::fsm::FsmCollection();
    finder
        .buildAppendFsm<peach::fsm::NameFinder>() //
        .buildAppendFsm<peach::fsm::OperatorFinder>(
            std::vector<std::pair<std::string, tokenCategory>>{
                {"!", tokenCategory::OPERATOR_UN},
                {"&", tokenCategory::OPERATOR_BI},
                {"|", tokenCategory::OPERATOR_BI},
                {"*", tokenCategory::OPERATOR_BI},
                {"/", tokenCategory::OPERATOR_BI},
                {"%", tokenCategory::OPERATOR_BI},
                {"+", tokenCategory::OPERATOR_BI},
                {"-", tokenCategory::OPERATOR_BI},
                {"==", tokenCategory::OPERATOR_BI},
                {"&=", tokenCategory::ASSIGNMENT},
                {"|=", tokenCategory::ASSIGNMENT},
                {"*=", tokenCategory::ASSIGNMENT},
                {"/=", tokenCategory::ASSIGNMENT},
                {"%=", tokenCategory::ASSIGNMENT},
                {"+=", tokenCategory::ASSIGNMENT},
                {"-=", tokenCategory::ASSIGNMENT},
                {"=", tokenCategory::ASSIGNMENT},
            }) //
        .buildAppendFsm<peach::fsm::SingleCharFinder>(
            std::vector<std::pair<char, tokenCategory>>{
                {'\n', tokenCategory::SEP_ENDL},
                {' ', tokenCategory::SEP_SPACE},
                {'\t', tokenCategory::SEP_TAB},
                {'(', tokenCategory::BRACKET_OPEN},
                {')', tokenCategory::BRACKET_CLOSE},
            }) //
        ;

    auto text = "whi=4\n"
                "whilel=4\n"
                "i = 0\n"
                "while (res != 0)\n"
                "    i = i + 1\n"
                "    res = res - 1\n"
                "    if i % 2 == 0:\n"
                "        res = res + a + b + i\n"
                "    elif i % 2 == 1:\n"
                "        res_2 = 0\n"
                "    else:\n"
                "        res = res + c - i\n"
                "res += c";

    // auto text = "+b";

    for (const auto &token : finder.tokenizeText(text, {
                                                           {"if", tokenCategory::COND_IF},
                                                           {"elif", tokenCategory::COND_ELIF},
                                                           {"else", tokenCategory::COND_ELSE},
                                                           {"while", tokenCategory::LOOP_WHILE},
                                                       }))
    {
        if (token->getCategory() != peach::token::tokenCategory::UNDEFINED)
        {
            std::string str = token->getTokenString();
            if (str == "\n")
            {
                str = "\\n";
            }
            std::cout << token->getPosition() << ' ' << '\'' << str << '\'' << ' ' << token->getCategory() << std::endl;
        }
    }

    return 0;
}

/*
res = 4
i = 0
while (res != 0)
    i = i + 1
    res = res - 1
    if i % 2 == 0:
        res = res + a + b + i
    elif i % 2 == 1:
        res = 0
    else:
        res = res + c - i
res += c
*/
