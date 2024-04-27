#include "brackets.hpp"

#include "doctest.h"

#include <cstdint>
#include <iostream>
#include <map>
#include <stack>
#include <string>
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

static const std::map<brackets::status, std::string> status_strings = {
    { brackets::status::ok, "OK"s },
    { brackets::status::stream_error, "Bad stream"s },
    { brackets::status::extra_close, "Close symbol without open"s },
    { brackets::status::left_open, "Reached end with unclosed symbols"s },
    { brackets::status::mismatch, "Mismatch"s },
};

std::string_view brackets::status_string(brackets::status s)
{
    return status_strings.at(s);
}

std::istream& operator>>(std::istream& is, brackets::code_reference& cr)
{
    if (is.get(cr.c))
    {
        if (cr.c == '\n')
        {
            ++cr.line;
            cr.column = 0;
        }
        else
        {
            ++cr.column;
        }
    }

    return is;
}

brackets::result brackets::check(std::istream& is)
{
    code_reference currentRef;

    if (!is)
    {
        return { status::stream_error, currentRef, {} };
    }

    std::stack<code_reference, std::vector<code_reference>> stack;

    while (is >> currentRef)
    {
        const auto it = matches_for.find(currentRef.c);
        if (it == matches_for.end()) // Not part of a pair
            ; // no-op
        else if (it->second == '\0') // open symbols are easy
            stack.push(currentRef);
        else if (stack.empty()) // close symbol with an empty stack
            return { status::extra_close, currentRef, {} };
        else if (stack.top().c != it->second) // wrong close symbol
            return { status::mismatch, currentRef, stack.top() };
        else // Right close symbol
            stack.pop();
    }

    if (!stack.empty())
        return { status::left_open, currentRef, stack.top() };

    return { status::ok, currentRef, {} };
}

TEST_CASE("Brackets matching")
{
    SUBCASE("correct")
    {
        const std::vector<std::tuple<std::string, std::string, brackets::code_reference>> data = {
            { "Empty input", "", { 0, 1 } },
            { "Sequential pairs", "()[]{}", { 6, 1 } },
            { "Nested pairs", "([{{()}}])", { 10, 1 } },
            { "Really mixed case", "(\n[]  {((\n\n)[]\n)})", { 3, 5 } },
        };

        for (const auto& [key, value, ref] : data)
        {
            CAPTURE(key);
            CAPTURE(value);
            std::istringstream is(value);
            const auto r = brackets::check(is);
            CHECK(r.code == brackets::status::ok);
            CHECK(r.error_ref.line == ref.line);
            CHECK(r.error_ref.column == ref.column);
        }
    }

    SUBCASE("mistakes")
    {
        const std::vector<std::tuple<std::string,
            std::string,
            brackets::status,
            brackets::code_reference>>
            data = {
	  { "Lone close"s, ")"s, brackets::status::extra_close, { 1, 1, ')' } },
	  { "Lone open"s, "["s, brackets::status::left_open, { 1, 1, '['} },
	  { "Simple mismatch"s, "{]"s, brackets::status::mismatch, { 2, 1, ']' } },
	  { "Mis-ordered pair"s, "}{"s, brackets::status::extra_close, { 1, 1, '}' } },
	  { "Mis-ordered nesting"s, "({)}"s, brackets::status::mismatch, { 3, 1, ')' } },
            };

        for (const auto& [key, value, code, ref] : data)
        {
            CAPTURE(key);
            CAPTURE(value);
            std::istringstream is(value);
            const auto r = brackets::check(is);
            CHECK(r.code == code);
            CHECK(r.error_ref.line == ref.line);
            CHECK(r.error_ref.column == ref.column);
	    CHECK(r.error_ref.c == ref.c);
        }
    }
}
