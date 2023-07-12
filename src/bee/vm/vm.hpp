#ifndef BEE_VM_HPP
#define BEE_VM_HPP

#include "core.hpp"
#include "error.hpp"
#include <fmt/core.h>

namespace bee
{

struct Ast;
struct Ast_Expr;
struct Unary_Expr;
struct Binary_Expr;
struct Nested_Expr;
struct Scope_Expr;
struct Id_Expr;

struct Vm
{
    Ast *ast;
    usize sp;
    u8 stack[1024];

    Vm(Ast *ast);
    void run();
    void run_expr(Ast_Expr *expr);
    void run_unary(Unary_Expr *unary);
    void run_binary(Binary_Expr *binary);
    void run_nested(Nested_Expr *nested);
    void run_scope(Scope_Expr *scope);
    void run_id(Id_Expr *id);

    u8 *stack_push(u8 *data, usize size);
    u8 *stack_pop(usize size);

    Error errorf(std::string_view fmt, auto... args)
    {
        return bee_errorf("vm error", fmt, args...);
    }
};

} // namespace bee

#endif
