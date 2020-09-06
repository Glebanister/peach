#include <fstream>

#include "PeachCli.hpp"

int main(int argc, char **argv)
{
    if (argc > 1)
    {
        auto infile = std::ifstream(argv[1]);
        peach::cli::PeachCli().executeProgram(infile, std::cout);
    }
    else
    {
        peach::cli::PeachCli().loop(std::cin, std::cout);
    }
}
