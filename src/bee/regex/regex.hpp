#ifndef BEE_REGEX_HPP
#define BEE_REGEX_HPP

#include "core.hpp"
#include "match.hpp"
#include "node.hpp"
#include "parser.hpp"

namespace bee::regex
{

struct Regex
{
    std::string_view src;
    Node *head;
    Node_Arena arena;

    Regex(std::string_view src) :
        src{src},
        arena(Node{
            State{.option = Regex_Monostate, .monostate{}},
            0,
            {},
        })
    {
        head = Parser{src, arena}.parse();
    }

    Regex(const char *src) : Regex(std::string_view{src}) {}

    Match match(std::string_view expr) const
    {
        return head != NULL ? Match{expr, head->submit(expr, 0)} : Match{expr, npos};
    }

    Match match(auto begin, auto end) const
    {
        return match(std::string_view{begin, end});
    }
};

inline regex::Regex operator""_rx(const char *src, usize)
{
    return Regex{src};
}

} // namespace bee::regex

namespace bee
{
using regex::Regex;
} // namespace bee

#endif
