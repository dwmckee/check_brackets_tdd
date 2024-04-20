// Bracket validation
#include "brackets.hpp"

#include <cstdlib>
#include <fstream>
#include <iostream>

int main(int argc, char* argv[])
{
    int status = EXIT_SUCCESS;

    for (int i = 0; i < argc; ++i)
    {
        std::ifstream f(argv[i]);
        if (brackets::check(f) != brackets::status::ok)
        {
            status = EXIT_FAILURE << 2;
            std::cerr << "Error in " << argv[i] << std::endl;
            break;
        }
    }

    return status;
}
