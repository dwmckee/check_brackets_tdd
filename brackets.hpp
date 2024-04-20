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

status check(std::istream& is);

}

#endif // BRACKETS_HPP
