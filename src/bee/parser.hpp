#ifndef BEE_PARSER_HPP
#define BEE_PARSER_HPP

#include "arena.hpp"
#include "ast.hpp"
#include "ast_dump.hpp"
#include "scanner.hpp"
#include "type_system.hpp"
#include <deque>
#include <unordered_set>

namespace bee
{

struct Parser
{
    Scanner &scanner;
    Ast ast;
    Type_System type_system;
    std::deque<Token> token_queue;
    std::deque<Ast_Expr *> stack;

    Parser(Scanner &scanner);
    void parse();

    Compound_Expr parse_compound(u64 sep_types, u64 end_types);
    Ast_Expr *parse_expr(u64 end_types);
    Ast_Expr *parse_one_expr(Ast_Expr *prev, u64 end_types);

    Unary_Expr *parse_increment(Token op, Ast_Expr *prev, u64 end_types);
    Ast_Expr *parse_condition(Token kw, Ast_Expr *expr);
    If_Expr *parse_if(Token kw, u64 end_types);
    Ast_Expr *parse_for(Token kw, u64 end_types);
    Scope_Expr *parse_scope(Ast_Frame *frame, u64 sep_types, u64 end_types);
    Ast_Entity *parse_type(Token token, u64 end_types);
    Binary_Expr *parse_binary_expr(Ast_Expr *prev, Ast_Expr *post, Token op);
    Ast_Expr *parse_def(Id_Expr *id, Token op, u64 end_types);
    Var_Expr *parse_var(Id_Expr *id, Token op, Ast_Expr *expr, Ast_Entity *type, u64 end_types);
    Function_Expr *parse_function(Id_Expr *id, Token op, Signature_Expr *signature, u64 end_types);
    Signature_Expr *parse_signature(Var_Expr *params, u64 end_types);
    Invoke_Expr *parse_invoke(Id_Expr *id, Token token);
    Argument_Expr *parse_argument(Var_Expr *param, Token token);

    Ast_Expr *stack_find(Ast_Expr_Kind kind) const;

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
