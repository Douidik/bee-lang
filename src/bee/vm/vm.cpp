#include "vm.hpp"
#include "ast.hpp"

namespace bee
{

Vm::Vm(Ast *ast) : ast{ast}, sp{0} {}

void Vm::run()
{
    for (Ast_Expr *expr : *ast->main_scope->compound)
    {
        run_expr(expr);
    }
}

void Vm::run_expr(Ast_Expr *expr)
{
    switch (expr->kind())
    {
    case Ast_Expr_Unary:
        return run_unary((Unary_Expr *)expr);

    case Ast_Expr_Binary:
    case Ast_Expr_Nested:
    case Ast_Expr_Scope:
    case Ast_Expr_Return:
    case Ast_Expr_Id:
    case Ast_Expr_Var:
    case Ast_Expr_Char:
    case Ast_Expr_Str:
    case Ast_Expr_Int:
    case Ast_Expr_Float:
    case Ast_Expr_Signature:
    case Ast_Expr_Function:
    case Ast_Expr_Argument:
    case Ast_Expr_Invoke:
    case Ast_Expr_If:
    case Ast_Expr_For:
    case Ast_Expr_For_Range:
    case Ast_Expr_For_While:

    default:
        break;
    }
}

void Vm::run_unary(Unary_Expr *unary)
{
    
    
    switch (unary->op.type)
    {
    case Token_Add: {
	
    }

    default:
        throw errorf("TODO! run_unary() not implemented for '{}'", token_typename(unary->op.type));
    }
}

void Vm::run_binary(Binary_Expr *binary) {}
void Vm::run_nested(Nested_Expr *nested) {}
void Vm::run_scope(Scope_Expr *scope) {}
void Vm::run_id(Id_Expr *id) {}

u8 *Vm::stack_push(u8 *data, usize size)
{
    if (sp + size > std::size(stack))
        throw errorf("stack overflow (sp > {})", std::size(stack));
    data = (u8 *)std::memcpy(&stack[sp], data, size);
    return sp += size, data;
}

u8 *Vm::stack_pop(usize size)
{
    if (sp - size < 0)
        throw errorf("stack underflow (sp < 0)");
    return &stack[sp -= size];
}

} // namespace bee
