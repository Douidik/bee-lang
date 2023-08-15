#ifndef BEE_VM_HPP
#define BEE_VM_HPP

#include "core.hpp"
#include "error.hpp"
#include "object.hpp"
#include <fmt/core.h>
#include <unordered_map>

namespace bee
{
struct Ast;
struct Ast_Expr;
struct Unary_Expr;
struct Binary_Expr;
struct Nested_Expr;
struct Scope_Expr;
struct Id_Expr;
struct Invoke_Expr;
struct Function_Expr;
struct Argument_Expr;
struct Return_Expr;
struct Var_Expr;
struct If_Expr;
struct Atom_Type;
struct Type_System;

struct Vm
{
    Ast *ast;
    Type_System &type_system;
    u64 sp;
    u8 stack[1024];
    Vm_Object vm_none;

    Vm(Ast *ast);
    s32 run();
    Vm_Object run_expr(Ast_Expr *expr);
    Vm_Object run_unary(Unary_Expr *unary);
    Vm_Object run_binary(Binary_Expr *binary);
    Vm_Object run_scope(Scope_Expr *scope);
    Vm_Object run_var(Var_Expr *def);
    Vm_Object run_var_id(Id_Expr *id);
    Vm_Object run_function(Function_Expr *def);
    Vm_Object run_invoke(Invoke_Expr *invoke);
    Vm_Object run_atom(Ast_Entity *entity, void *data);
    Vm_Object run_return(Return_Expr *return_expr);
    Vm_Object run_if(If_Expr *if_expr);

    void init_params(Var_Expr *param, Argument_Expr *argument);
    u8 *expr_source(Ast_Expr *expr);
    Vm_Atom vm_atom(Atom_Type *type, u8 *source);

    u8 *stack_push(u8 *data, usize size);
    u8 *stack_pop(usize size);

    Error errorf(std::string_view fmt, auto... args)
    {
        return Error{"vm error", fmt::format(fmt::runtime(fmt), args...)};
    }
};

} // namespace bee

#endif
