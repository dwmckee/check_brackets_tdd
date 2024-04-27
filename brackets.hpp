#ifndef BRACKETS_HPP
#define BRACKETS_HPP

#include <iosfwd>
#include <string_view>

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

std::string_view status_string(status s);

struct code_reference
{
  size_t column = 0;
  size_t line = 1;
  char c = '\0';
};
  
struct result
{
  status code = status::ok;
  code_reference error_ref;
  code_reference back_ref;
};

result check(std::istream& is);
}

#endif // BRACKETS_HPP
