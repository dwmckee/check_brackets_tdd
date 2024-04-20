#ifndef BRACKETS_HPP
#define BRACKETS_HPP

#include <iosfwd>

namespace brackets
{

enum class status
{
    ok,
    stream_error,
    extra_close,
    left_open,
    mismatch
};

struct result
{
    status code = status::ok;
    size_t column = 0;
    size_t line = 1;
};

result check(std::istream& is);
}

#endif // BRACKETS_HPP
