// Bracket validation
#include "brackets.hpp"

#include <cstdlib>
#include <fstream>
#include <iostream>

int main(int argc, char* argv[])
{
    int final_status = EXIT_SUCCESS;

    for (int i = 1; i < argc; ++i)
    {
        std::ifstream f(argv[i]);
        auto status = brackets::check(f);
        if (status != brackets::status::ok)
        {
            final_status = EXIT_FAILURE;
            std::cerr << "Error in " << argv[i] << std::endl;
        }
    }

    return final_status;
}
