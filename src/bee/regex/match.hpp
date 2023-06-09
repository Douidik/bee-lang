#ifndef BEE_REGEX_MATCH_HPP
#define BEE_REGEX_MATCH_HPP

#include "core.hpp"

namespace bee::regex
{

struct Match
{
    std::string_view expr;
    usize index;

    std::string_view view() const;
    std::string_view next() const;
    const char *begin() const;
    const char *end() const;
    operator bool() const;
};

} // namespace bee::regex

#endif
