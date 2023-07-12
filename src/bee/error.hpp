#ifndef BEE_ERROR_HPP
#define BEE_ERROR_HPP

#include <exception>
#include <string>
#include <string_view>

namespace bee
{

struct Error : std::exception
{
    std::string_view name;
    std::string buffer;
    
    Error(std::string_view name, std::string buffer) : name{name}, buffer{buffer} {}

    const char *what() const noexcept override
    {
        return buffer.c_str();
    }
};

} // namespace bee

#endif
