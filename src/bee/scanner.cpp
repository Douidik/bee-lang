#include "scanner.hpp"

namespace bee
{

Scanner::Scanner(std::string_view src, Syntax_Map map) : source{src}, next{src}, map{map}
{
    if (!src.ends_with('\n'))
    {
        throw errorf(dummy_token(Token_Eof), "source does not ends with a endline character '\\n'");
    }
}

Token Scanner::tokenize()
{
    Token token = {};
    do
    {
        if (next.empty())
        {
            return dummy_token(Token_Eof);
        }

        for (const auto &[type, regex] : map)
        {
            if (regex::Match match = regex.match(next))
            {
                next = match.next();
                token.expr = match.view();
                token.type = type;
                break;
            }
        }
    } while (token.type & (Token_Blank | Token_Comment));

    if (!token.type)
    {
        throw errorf(token, "unrecognized token");
    }

    return token;
}

Token Scanner::dummy_token(Token_Type type) const
{
    return Token{std::string_view{&source.back(), 1}, type, true};
}

bool Scanner::eof() const
{
    return next.empty();
}

} // namespace bee
