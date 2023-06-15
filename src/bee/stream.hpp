#ifndef BEE_STREAM_HPP
#define BEE_STREAM_HPP

#include "core.hpp"
#include <fmt/format.h>
#include <fmt/os.h>
#include <ostream>

namespace bee
{

struct Standard_Stream
{
    fmt::memory_buffer buffer;

    Standard_Stream() : buffer() {}

    std::string_view str() const
    {
        return std::string_view{buffer.begin(), buffer.end()};
    }

    Standard_Stream &std_print(std::string_view fmt, auto... args)
    {
        fmt::format_to(std::back_inserter(buffer), fmt::runtime(fmt), args...);
        return *this;
    }
};

struct Stream : Standard_Stream
{
    Stream &print(std::string_view fmt, auto... args)
    {
        std_print(fmt, args...);
        return *this;
    }
};

} // namespace bee

#endif
