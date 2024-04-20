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
        const auto result = brackets::check(f);
        if (result.code != brackets::status::ok)
        {
            final_status = EXIT_FAILURE;
            std::cerr << "Error: " << argv[i]
                      << ":" << result.line
                      << ":" << result.column << std::endl;
        }
    }

    return final_status;
}
