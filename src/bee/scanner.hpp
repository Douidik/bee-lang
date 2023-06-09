#ifndef BEE_SCANNER_HPP
#define BEE_SCANNER_HPP

#include "bee_error.hpp"
#include "token.hpp"

namespace bee
{

struct Scanner
{
    std::string_view src;
    std::string_view next;
    Syntax_Map map;

    Scanner(std::string_view src, Syntax_Map map);

    Token tokenize();
    Token dummy_token(Token_Type type) const;
    bool eof() const;

    Error errorf(Token token, std::string_view fmt, auto... args)
    {
        return bee_errorf("scanner error", src, token, fmt, args...);
    }
};

} // namespace bee

#endif
