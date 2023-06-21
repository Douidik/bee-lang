#ifndef BEE_STREAM_HPP
#define BEE_STREAM_HPP

#include "core.hpp"
#include <fmt/format.h>
#include <fmt/os.h>
#include <ostream>

namespace bee
{

struct Stream
{
    fmt::memory_buffer buffer;

    Stream() = default;
    
    std::string_view str() const
    {
        return std::string_view{buffer.begin(), buffer.end()};
    }

    Stream &print(std::string_view fmt, auto... args)
    {
        fmt::format_to(std::back_inserter(buffer), fmt::runtime(fmt), args...);
        return *this;
    }
};

} // namespace bee

#endif
