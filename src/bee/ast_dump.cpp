#include "ast_dump.hpp"
#include "expr.hpp"

namespace bee
{

void Ast_Dump::dump_expr(Ast_Expr *ast_expr, s32 depth)
{
    Ast_Expr_Kind kind = ast_expr->kind();

    switch (kind)
    {
    case Ast_Expr_Unary: {
        auto &[op, order, expr] = *(Unary_Expr *)ast_expr;
        print(depth, "Unary_Expr (op: '{:s}', order: '{:s}')\n", op.expr, order_expr_name(order));
        break;
    }

    case Ast_Expr_Binary: {
        auto &[op, prev, post] = *(Binary_Expr *)ast_expr;
        print(depth, "Binary_Expr (op: '{:s}')\n", op.expr);
        dump_expr(prev, depth + 1), dump_expr(post, depth + 1);
        break;
    }

    case Ast_Expr_Nested: {
        Nested_Expr *nested = (Nested_Expr *)ast_expr;
        print(depth, "Nested_Expr\n");
        dump_expr(nested->expr, depth + 1);
        break;
    }

    case Ast_Expr_Scope: {
        Scope_Expr *scope = (Scope_Expr *)ast_expr;
        print(depth, "Scope_Expr\n");
        for (Ast_Expr *expr : scope->body)
        {
            dump_expr(expr, depth + 1);
        }
        break;
    }

    case Ast_Expr_Id: {
        Id_Expr *id = (Id_Expr *)ast_expr;
        print(depth, "Expr_Id\n");
        dump_entity(id->var, depth + 1);
        break;
    }

    case Ast_Expr_Def: {
    }

    case Ast_Expr_Char:
    case Ast_Expr_Str:
    case Ast_Expr_Int:
    case Ast_Expr_Float:
    case Ast_Expr_Proc:
    case Ast_Expr_Invoke:
        break;

    default: {
        print(depth, "TODO! dump_expr() for {:s}", ast_expr_kind_name(kind));
        break;
    }
    }
}

Ast_Dump &Ast_Dump::dump_entity(Ast_Entity *ast_expr, s32 depth) const {}

} // namespace bee
