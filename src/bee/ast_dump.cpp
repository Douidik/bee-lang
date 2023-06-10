#include "ast_dump.hpp"
#include "ast.hpp"
#include "escape_sequence.hpp"
#include "expr.hpp"

namespace bee
{

Ast_Dump::Ast_Dump(Ast &ast)
{
    dump_expr(ast.exprs.back(), 0);
}

void Ast_Dump::dump_expr(Ast_Expr *ast_expr, s32 depth)
{
    if (!ast_expr)
    {
        print(depth, "(NULL)\n");
        return;
    }

    switch (ast_expr->kind)
    {
    case Ast_Expr_Unary: {
        auto &[op, order, expr] = ast_expr->unary_expr;
        print(depth, "Unary_Expr (op: '{:s}', order: '{:s}')\n", op.expr, order_expr_name(order));
        break;
    }

    case Ast_Expr_Binary: {
        auto &[op, prev, post] = ast_expr->binary_expr;
        print(depth, "Binary_Expr (op: '{:s}')\n", op.expr);
        dump_expr(prev, depth + 1), dump_expr(post, depth + 1);
        break;
    }

    case Ast_Expr_Nested: {
        Nested_Expr *nested = &ast_expr->nested_expr;
        print(depth, "Nested_Expr\n");
        dump_expr(nested->expr, depth + 1);
        break;
    }

    case Ast_Expr_Scope: {
        Scope_Expr *scope = &ast_expr->scope_expr;
        print(depth, "Scope_Expr\n");
        for (Ast_Expr *expr : scope->body)
        {
            dump_expr(expr, depth + 1);
        }
        break;
    }

    case Ast_Expr_Id: {
        Id_Expr *id = &ast_expr->id_expr;
        print(depth, "Id_Expr\n");
        dump_entity(id->entity, depth + 1);
        break;
    }

    case Ast_Expr_Def: {
        auto &[entity, expr, op, name] = ast_expr->def_expr;
        print(depth, "Def_Expr (op: '{:s}', name: '{:s}')\n", op.expr, name.expr);
        dump_expr(expr, depth + 1);
        dump_entity(entity, depth + 1);
        break;
    }

    case Ast_Expr_Char: {
        auto &[c] = ast_expr->char_expr;
        print(depth, "Char_Expr (data: '{:s}')\n", escape_sequence(c));
        break;
    }

    case Ast_Expr_Str: {
        auto &[s] = ast_expr->str_expr;
        print(depth, "Str_Expr (data: '{:s}')\n", escape_string(s));
        break;
    }

    case Ast_Expr_Int: {
        auto &[data, size] = ast_expr->int_expr;
        print(depth, "Int_Expr (data: {:d}, size: {:d})\n", data, size);
        break;
    }

    case Ast_Expr_Float: {
        auto &[data, size] = ast_expr->float_expr;
        print(depth, "Float_Expr (data: {:f}, size: {:d})\n", data, size);
        break;
    }

    default: {
        print(depth, "TODO! dump_expr() for {:s}\n", ast_expr_kind_name(ast_expr->kind));
        break;
    }
    }
}

void Ast_Dump::dump_entity(Ast_Entity *ast_entity, s32 depth)
{
    if (!ast_entity)
    {
        print(depth, "(NULL)\n");
        return;
    }

    switch (ast_entity->kind)
    {
    case Ast_Entity_Var: {
        print(depth, "Var\n");
        dump_entity(ast_entity->var.type, depth + 1);
        break;
    }

    case Ast_Entity_Void: {
        print(depth, "Void_Type\n");
        break;
    }

    case Ast_Entity_Atom: {
        auto &[desc, size] = ast_entity->atom_type;
        print(depth, "Atom_Type (desc: {:s}, size: {:d})\n", atom_desc_name(desc), size);
        break;
    }

    default: {
        print(depth, "TODO! dump_entity() for {:s}\n", ast_entity_kind_name(ast_entity->kind));
        break;
    }
    }
}

} // namespace bee
