#ifndef BEE_PARSER_HPP
#define BEE_PARSER_HPP

#include "arena.hpp"
#include "ast.hpp"
#include "scanner.hpp"
#include <deque>

namespace bee
{
    
struct Parser
{
    Scanner &scanner;
    Ast ast;
    std::deque<Token> token_queue;

    Parser(Scanner &scanner);
    Ast parse();

    std::vector<Ast_Expr *> parse_compound(Token_Type sep_type, Token_Type end_type);
    Ast_Expr *parse_expr(Token_Type end_type);
    Ast_Expr *parse_expected_expr(Ast_Expr *prev, Token_Type end_type);
    
    Token scan(Token_Type type);
    Token scan(std::initializer_list<Token_Type> types);

    void def_standard_types(Frame *frame);

    Error errorf(Token token, std::string_view fmt, auto... args) const
    {
        return bee_errorf("parser error", scanner.src, token, fmt, args...);
    }
};

} // namespace bee

#endif
