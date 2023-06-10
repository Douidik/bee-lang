#ifndef BEE_PARSER_HPP
#define BEE_PARSER_HPP

#include "arena.hpp"
#include "ast.hpp"
#include "scanner.hpp"
#include "type_system.hpp"
#include <deque>

namespace bee
{

using Token_Types = std::initializer_list<Token_Type>;

struct Parser
{
    Scanner &scanner;
    Ast ast;
    Type_System type_system;
    std::deque<Token> token_queue;

    Parser(Scanner &scanner);
    void parse();

    std::vector<Ast_Expr *> parse_compound(Token_Types sep_types, Token_Types end_types);
    Ast_Expr *parse_expr(Token_Types end_types);
    Ast_Expr *parse_expected_expr(Ast_Expr *prev, Token_Types end_types);
    Ast_Expr *parse_def(Ast_Expr *prev, Token op, Token_Types end_types);

    bool eof() const;
    Token peek();
    Token scan(Token_Types types);

    Error error_expected(Token token, Token_Types types) const;

    Error errorf(Token token, std::string_view fmt, auto... args) const
    {
        return bee_errorf("parser error", scanner.src, token, fmt, args...);
    }
};

} // namespace bee

#endif
