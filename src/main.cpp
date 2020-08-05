#include <algorithm>
#include <iostream>
#include <memory>
#include <numeric>
#include <unordered_map>
#include <vector>

#include "NameFinder.hpp"

int main()
{
    auto finder = peach::fsm::NameFinder();
    auto text = "res = 4\n"
                "i = 0\n"
                "while (res != 0)\n"
                "    i = i + 1\n"
                "    res = res - 1\n"
                "    if i % 2 == 0:\n"
                "        res = res + a + b + i\n"
                "    elif i % 2 == 1:\n"
                "        res = 0\n"
                "    else:\n"
                "        res = res + c - i\n"
                "res += c";
    for (const auto &token : finder.tokenizeText(text, {{"while", peach::token::tokenCategory::LOOP_WHILE},
                                                        {"if", peach::token::tokenCategory::COND_IF},
                                                        {"elif", peach::token::tokenCategory::COND_ELIF},
                                                        {"else", peach::token::tokenCategory::COND_ELSE}}))
    {
        std::cout << token->getPosition() << " '" << token->getTokenString() << "' " << static_cast<int>(token->getCategory()) << std::endl;
    }
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
