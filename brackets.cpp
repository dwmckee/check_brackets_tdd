#include "brackets.hpp"

#include "doctest.h"

#include <cstdint>
#include <iostream>
#include <map>
#include <stack>
#include <vector>
// Needed for testing, not for the main implementation
#include <sstream>
#include <type_traits>
using namespace std::string_literals;

// All characters to process are here. If the value character is non-null it
// represents the one open the key (which is a close) can consume.
static const std::map<char, char> matches_for = {
    { '(', '\0' }, { '[', '\0' }, { '{', '\0' },
    { ')', '(' }, { ']', '[' }, { '}', '{' }
};

brackets::status brackets::check(std::istream& is)
{
    if (!is)
        return status::stream_error;

    std::stack<char, std::vector<char>> stack;

    std::istream::char_type c;
    while (is.get(c))
    {
        const auto it = matches_for.find(c);
        if (it != matches_for.end())
        {
            if (it->second == '\0')
                stack.push(c);
            else if (stack.empty())
                return status::extra_close;
            else if (stack.top() == it->second)
                stack.pop();
            else
                return status::mismatch;
        }
    }

    if (!stack.empty())
        return status::left_open;

    return status::ok;
}

TEST_CASE("Brackets matching")
{
    SUBCASE("correct")
    {
        std::map<std::string, std::string> data = {
            { "Empty input", "" },
            { "Sequential pairs", "()[]{}" },
            { "Nested pairs", "([{{()}}])" },
            { "Really mixed case", "([]{(()[])})" },
        };

        for (const auto& [key, value] : data)
        {
            CAPTURE(key);
            CAPTURE(value);
            std::istringstream is(value);
            CHECK(brackets::check(is) == brackets::status::ok);
        }
    }

    SUBCASE("mistakes")
    {
        const std::vector<std::tuple<std::string,
            std::string,
            brackets::status>>
            data = {
                { "Lone close"s, ")"s, brackets::status::extra_close },
                { "Lone open"s, "["s, brackets::status::left_open },
                { "Simple mismatch"s, "{]"s, brackets::status::mismatch },
                { "Mis-ordered pair"s, "}{"s, brackets::status::extra_close },
                { "Mis-ordered nesting"s, "({)}"s, brackets::status::mismatch },
            };

        for (const auto& [key,
                 value,
                 expected_result] : data)
        {
            CAPTURE(key);
            CAPTURE(value);
            std::istringstream is(value);
            CHECK(brackets::check(is) == expected_result);
        }
    }
}
