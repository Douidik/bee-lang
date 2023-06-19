#include "ast_dump.hpp"
#include "ast.hpp"
#include "escape_sequence.hpp"
#include "expr.hpp"
#include "function.hpp"
#include "type.hpp"
#include "var.hpp"

namespace bee
{

s32 stack_depth(Ast_Frame *frame)
{
    return 1 + (frame->owner ? stack_depth(frame->owner) : 0);
}

Ast_Dump::Ast_Dump(Ast &ast)
{
    print(0, "Ast_Stack\n");
    stack_dump(ast.frame, stack_depth(ast.frame));
    print(0, "Ast_Expressions\n");
    expr_dump(ast.main_scope, 1);
}

void Ast_Dump::stack_dump(Ast_Frame *frame, s32 depth)
{
    if (!frame)
        return;

    for (auto &[name, entity] : frame->defs)
    {
        entity_dump(entity, depth);
    }

    stack_dump(frame->owner, depth - 1);
}

void Ast_Dump::expr_dump(Ast_Expr *ast_expr, s32 depth)
{
    if (!ast_expr)
    {
        print(depth, "(NULL)\n");
        return;
    }

    switch (ast_expr->kind())
    {
    case Ast_Expr_None: {
        print(depth, "None_Expr\n");
        break;
    }

    case Ast_Expr_Unary: {
        auto &[op, order, expr] = *(Unary_Expr *)ast_expr;
        print(depth, "Unary_Expr (op: '{:s}', order: '{:s}')\n", op.expr, order_expr_name(order));
        break;
    }

    case Ast_Expr_Binary: {
        auto &[op, type, prev, post] = *(Binary_Expr *)ast_expr;
        print(depth, "Binary_Expr (op: '{:s}')\n", op.expr);
        expr_dump(prev, depth + 1), expr_dump(post, depth + 1);
        entity_dump(type, depth + 1);
        break;
    }

    case Ast_Expr_Nested: {
        Nested_Expr *nested = (Nested_Expr *)ast_expr;
        print(depth, "Nested_Expr\n");
        expr_dump(nested->expr, depth + 1);
        break;
    }

    case Ast_Expr_Scope: {
        Scope_Expr *scope = (Scope_Expr *)ast_expr;
        print(depth, "Scope_Expr\n");
        if (scope->compound != NULL and !scope->compound->empty())
        {
            for (Ast_Expr *expr : *scope->compound)
            {
                expr_dump(expr, depth + 1);
            }
        }
        else
        {
            print(depth + 1, "(Empty scope)\n");
        }

        break;
    }

    case Ast_Expr_If: {
        auto &[cond, scope] = *(If_Expr *)ast_expr;
        print(depth, "If_Expr\n");
        expr_dump(cond, depth + 1);
        expr_dump(scope, depth + 1);
        break;
    }

    case Ast_Expr_Return: {
        Return_Expr *return_expr = (Return_Expr *)ast_expr;
        print(depth, "Return_Expr\n");
        expr_dump(return_expr->expr, depth + 1);
        break;
    }

    case Ast_Expr_Id: {
        Id_Expr *id = (Id_Expr *)ast_expr;
        print(depth, "Id_Expr\n");
        entity_dump(id->entity, depth + 1);
        break;
    }

    case Ast_Expr_Var: {
        auto &[entity, expr, op, name, next] = *(Var_Expr *)ast_expr;
        print(depth, "Def_Expr (op: '{:s}', name: '{:s}')\n", op.expr, name.expr);
        expr_dump(expr, depth + 1);
        entity_dump(entity, depth + 1);
        if (next != NULL)
            expr_dump(next, depth + 1);
        break;
    }

    case Ast_Expr_Char: {
        auto &[c] = *(Char_Expr *)ast_expr;
        print(depth, "Char_Expr (data: '{:s}')\n", escape_sequence(c));
        break;
    }

    case Ast_Expr_Str: {
        auto &[s] = *(Str_Expr *)ast_expr;
        print(depth, "Str_Expr (data: '{:s}')\n", escape_string(s));
        break;
    }

    case Ast_Expr_Int: {
        auto &[data, size] = *(Int_Expr *)ast_expr;
        print(depth, "Int_Expr (data: {:d}, size: {:d})\n", data, size);
        break;
    }

    case Ast_Expr_Float: {
        auto &[data, size] = *(Float_Expr *)ast_expr;
        print(depth, "Float_Expr (data: {:f}, size: {:d})\n", data, size);
        break;
    }

    case Ast_Expr_Signature: {
        Signature_Expr *signature = (Signature_Expr *)ast_expr;
        print(depth, "Signature_Expr\n");
        expr_dump(signature->params, depth + 1);
        entity_dump(signature->type, depth + 1);
        break;
    }

    case Ast_Expr_Function: {
        Function_Expr *function = (Function_Expr *)ast_expr;
        print(depth, "Function_Expr\n");
        entity_dump(function->function, depth + 1);
        expr_dump(function->scope, depth + 1);
        break;
    }

    case Ast_Expr_Argument: {
        auto &[expr, next] = *(Argument_Expr *)ast_expr;
        print(depth, "Argument_Expr\n");
        expr_dump(expr, depth + 1);
        expr_dump(next, depth + 1);
        break;
    }

    case Ast_Expr_Invoke: {
        auto &[function, args] = *(Invoke_Expr *)ast_expr;
        print(depth, "Invoke_Expr\n");
        entity_dump(function, depth + 1);
        expr_dump(args, depth + 1);
        break;
    }

    default: {
        print(depth, "TODO! dump_expr() for {:s}\n", ast_expr_kind_name(ast_expr->kind()));
        break;
    }
    }
}

void Ast_Dump::entity_dump(Ast_Entity *ast_entity, s32 depth)
{
    if (!ast_entity)
    {
        print(depth, "(NULL)\n");
        return;
    }

    switch (ast_entity->kind())
    {
    case Ast_Entity_Var: {
        Var *var = (Var *)ast_entity;
        print(depth, "Var (name: '{}')\n", ast_entity->name);
        entity_dump(var->type, depth + 1);
        break;
    }

    case Ast_Entity_Function: {
        Function *function = (Function *)ast_entity;
        print(depth, "Function '{:s}'\n", function->repr());
        break;
    }

    case Ast_Entity_Void: {
        print(depth, "Void_Type\n");
        break;
    }

    case Ast_Entity_Atom: {
        Atom_Type *atom = (Atom_Type *)ast_entity;
        print(depth, "Atom_Type (name: '{}', desc: {:s}, size: {:d})\n", atom->name, atom_desc_name(atom->desc),
              atom->size);
        break;
    }

    default: {
        print(depth, "TODO! dump_entity() for {:s}\n", ast_entity_kind_name(ast_entity->kind()));
        break;
    }
    }
}

} // namespace bee
