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

brackets::result brackets::check(std::istream& is)
{
    result theResult;

    if (!is)
    {
        theResult.code = status::stream_error;
        return theResult;
    }

    std::stack<char, std::vector<char>> stack;

    std::istream::char_type c;
    while (is.get(c))
    {
        if (c == '\n')
        {
            ++theResult.line;
            theResult.column = 0;
        }
        else
            ++theResult.column;

        const auto it = matches_for.find(c);
        if (it != matches_for.end())
        {
            if (it->second == '\0') // open symbols are easy
                stack.push(c);
            else if (stack.empty()) // close symbol with an empty stack
            {
                theResult.code = status::extra_close;
                return theResult;
            }
            else if (stack.top() == it->second) // correct pending open
                stack.pop();
            else
            {
                theResult.code = status::mismatch;
                return theResult;
            }
        }
    }

    if (!stack.empty())
        theResult.code = status::left_open;

    return theResult;
}

TEST_CASE("Brackets matching")
{
    SUBCASE("correct")
    {
        const std::vector<std::tuple<std::string, std::string, size_t, size_t>> data = {
            { "Empty input", "", 1, 0 },
            { "Sequential pairs", "()[]{}", 1, 6 },
            { "Nested pairs", "([{{()}}])", 1, 10 },
            { "Really mixed case", "(\n[]  {((\n\n)[]\n)})", 5, 3 },
        };

        for (const auto& [key, value, line, column] : data)
        {
            CAPTURE(key);
            CAPTURE(value);
            std::istringstream is(value);
            const auto r = brackets::check(is);
            CHECK(r.code == brackets::status::ok);
            CHECK(r.line == line);
            CHECK(r.column == column);
        }
    }

    SUBCASE("mistakes")
    {
        const std::vector<std::tuple<std::string,
            std::string,
            brackets::status,
            size_t,
            size_t>>
            data = {
                { "Lone close"s, ")"s, brackets::status::extra_close, 1, 1 },
                { "Lone open"s, "["s, brackets::status::left_open, 1, 1 },
                { "Simple mismatch"s, "{]"s, brackets::status::mismatch, 1, 2 },
                { "Mis-ordered pair"s, "}{"s, brackets::status::extra_close, 1, 1 },
                { "Mis-ordered nesting"s, "({)}"s, brackets::status::mismatch, 1, 3 },
            };

        for (const auto& [key, value, expected_result, line, column] : data)
        {
            CAPTURE(key);
            CAPTURE(value);
            std::istringstream is(value);
            const auto r = brackets::check(is);
            CHECK(r.code == expected_result);
            CHECK(r.line == line);
            CHECK(r.column == column);
        }
    }
}
