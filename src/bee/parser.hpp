#ifndef BEE_PARSER_HPP
#define BEE_PARSER_HPP

#include "arena.hpp"
#include "ast.hpp"
#include "scanner.hpp"
#include "type_system.hpp"
#include <deque>
#include <unordered_set>

#include "ast_dump.hpp"

namespace bee
{

struct Parser
{
    Scanner &scanner;
    Ast ast;
    Type_System type_system;
    std::deque<Token> token_queue;

    Parser(Scanner &scanner);
    void parse();

    Compound_Expr parse_compound(u64 sep_types, u64 end_types);
    Ast_Expr *parse_expr(u64 end_types);
    Ast_Expr *parse_one_expr(Ast_Expr *prev, u64 end_types);

    Ast_Expr *parse_scope(u64 sep_types, u64 end_types);
    Ast_Entity *parse_type(Token token, u64 end_types);

    Ast_Expr *parse_def(Ast_Expr *prev, Token op, u64 end_types);
    Ast_Expr *parse_proc(Ast_Expr *params, u64 end_types);
    Ast_Expr *parse_invoke(Ast_Expr *proc, Token token);
    Ast_Expr *parse_argument(Signature_Type *signature, Ast_Expr *param, Token token);

    bool eof() const;
    Token peek(u64 types);
    Token scan(u64 types);
    
    Error error_expected(Token token, u64 types);

    Error errorf(Token token, std::string_view fmt, auto... args)
    {
        fmt::print("{}", Ast_Dump{ast}.str());
        return bee_errorf("parser error", scanner.src, token, fmt, args...);
    }
};

} // namespace bee

#endif
