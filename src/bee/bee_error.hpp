#ifndef BEE_BEE_ERROR_HPP
#define BEE_BEE_ERROR_HPP

#include "core.hpp"
#include "error.hpp"
#include "token.hpp"
#include <algorithm>

namespace bee
{

Error bee_errorf(std::string_view name, std::string_view src, Token token, std::string_view fmt, auto... args)
{
    std::string_view expr = token.expr;

    auto line = std::count(src.begin(), expr.begin(), '\n');
    auto rbegin = std::find(expr.rend(), src.rend(), '\n');
    auto begin = std::max(rbegin.base(), src.begin());
    auto end = std::find(expr.end(), src.end(), '\n');
    auto cursor = expr.begin() - begin + 1;
    auto desc = fmt::format(fmt::runtime(fmt), args...);

    return Error{
        name,
        fmt::format(R"(with {{
  {} | {}
     {:>{}}{:^>{}} {}
}})",
                    line, std::string_view{begin, end}, "", cursor, "^", expr.size(), desc),
    };
}

} // namespace bee

#endif
