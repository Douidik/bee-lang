#ifndef BEE_REGEX_STATE_HPP
#define BEE_REGEX_STATE_HPP

#include "core.hpp"

namespace bee::regex
{

struct Node;

enum Option : u32
{
    Regex_Monostate,
    Regex_Eps,
    Regex_Any,
    Regex_None,
    Regex_Not,
    Regex_Dash,
    Regex_Str,
    Regex_Set,
    Regex_Scope,
};

struct Monostate
{
};

struct State
{
    Option option;
    union {
        Monostate monostate;
        char range[2];
        std::string_view str;
        Node *sequence;
    };

    usize submit(std::string_view expr, usize n) const;
};

} // namespace bee::regex

#endif
