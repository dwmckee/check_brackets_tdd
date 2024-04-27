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
    { brackets::status::left_open, "Reached end with open pair(s)"s },
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

std::string reference_location(const brackets::code_reference& ref,
    std::string_view streamID)
{
    std::ostringstream ss;
    ss << streamID << ":" << ref.line << ":" << ref.column;
    return ss.str();
}

std::string brackets::result_report(const result& res, std::string_view streamID)
{
    constexpr auto eStr("error: ");

    std::ostringstream ss;
    ss << reference_location(res.error_ref, streamID)
       << ": "
       << (res.code == status::ok ? "" : eStr)
       << status_string(res.code) << ": ";
    switch (res.code)
    {
    case status::ok:
    case status::stream_error:
        // No additional data needed.
        break;
    case status::extra_close:
        ss << "Found '" << res.error_ref.c
           << "' with no matching (or, indeed, un-matching) open symbol.";
        break;
    case status::left_open:
        ss << "Found end-of-file with open symbol '"
           << res.back_ref.c << "' at "
           << reference_location(res.back_ref, streamID) << ".";
        break;
    case status::mismatch:
        ss << "Found '" << res.error_ref.c
           << "' as putative match for '" << res.back_ref.c << "' at "
           << reference_location(res.back_ref, streamID) << ".";
        break;
    }

    return ss.str();
}

TEST_CASE("result reporting")
{
    using namespace brackets;

    constexpr code_reference ref1 { 2, 2, 'A' };
    constexpr code_reference ref2 { 1, 1, 'B' };
    constexpr auto id = "ID";

    const std::vector<std::tuple<result, std::string>> data = {
        { { status::ok, ref1, ref2 }, "ID:2:2: OK: " },
        { { status::stream_error, ref1, ref2 },
            "ID:2:2: error: Bad stream: " },
        { { status::extra_close, ref1, ref2 },
            "ID:2:2: error: Close symbol without open: Found 'A' with no matching (or, indeed, un-matching) open symbol." },
        { { status::left_open, ref1, ref2 },
            "ID:2:2: error: Reached end with open pair(s): Found end-of-file with open symbol 'B' at ID:1:1." },
        { { status::mismatch, ref1, ref2 },
            "ID:2:2: error: Mismatch: Found 'A' as putative match for 'B' at ID:1:1." }
    };

    for (const auto& [res, expected] : data)
    {
        CAPTURE(res.code);
        const auto actual = result_report(res, id);
        CHECK(actual == expected);
    }
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
        using namespace brackets;

        const std::vector<std::tuple<std::string, std::string, code_reference>> data = {
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
            const auto r = check(is);
            CHECK(r.code == status::ok);
            CHECK(r.error_ref.line == ref.line);
            CHECK(r.error_ref.column == ref.column);
        }
    }

    SUBCASE("mistakes")
    {
        using namespace brackets;

        const std::vector<std::tuple<std::string,
            std::string,
            result>>
            data = {
                { "Lone close"s, ")"s, { status::extra_close, { 1, 1, ')' } } },
                { "Lone open"s, "["s, { status::left_open, { 1, 1, '[' }, { 1, 1, '[' } } },
                { "Simple mismatch"s, "{]"s, { status::mismatch, { 2, 1, ']' }, { 1, 1, '{' } } },
                { "Mis-ordered pair"s, "}{"s, { status::extra_close, { 1, 1, '}' } } },
                { "Mis-ordered nesting"s, "({)}"s, { status::mismatch, { 3, 1, ')' }, { 2, 1, '{' } } },
            };

        for (const auto& [key, value, expected] : data)
        {
            CAPTURE(key);
            CAPTURE(value);
            std::istringstream is(value);
            const auto r = check(is);
            CHECK(r.code == expected.code);
            CHECK(r.error_ref.line == expected.error_ref.line);
            CHECK(r.error_ref.column == expected.error_ref.column);
            CHECK(r.error_ref.c == expected.error_ref.c);
            CHECK(r.back_ref.line == expected.back_ref.line);
            CHECK(r.back_ref.column == expected.back_ref.column);
            CHECK(r.back_ref.c == expected.back_ref.c);
        }
    }
}
