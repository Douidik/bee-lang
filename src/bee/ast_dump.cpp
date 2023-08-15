#include "ast_dump.hpp"
#include "ast.hpp"
#include "function.hpp"
#include "var.hpp"

namespace bee
{

s32 frame_depth(Frame *frame)
{
    return frame ? frame_depth(frame->owner) + 1 : 0;
}

Ast_Dump::Ast_Dump(Ast *ast) : ast(ast)
{
    frame_dump({"main_frame", 0}, ast->main_frame);
    expr_dump({"main_scope", 0}, ast->main_scope);
}

void Ast_Dump::frame_dump(Ast_Dump_Header h, Frame *frame)
{
    if (!frame or frame->defs.empty())
    {
        print("{} (empty-frame)\n", h);
        return;
    }
    print("{}\n", h);
    for (auto &[name, entity] : frame->defs)
        entity_dump({"", h.depth + 1}, entity);
}

void Ast_Dump::expr_dump(Ast_Dump_Header h, Ast_Expr *ast_expr)
{
    if (!ast_expr)
    {
        print("{} (none)\n", h);
        return;
    }

    switch (ast_expr->kind())
    {
    case Ast_Expr_Unary: {
        Unary_Expr *unary = (Unary_Expr *)ast_expr;
        print("{} unary-expr [op: '{:s}', order: '{:s}']\n", h, unary->op.expr, order_expr_name(unary->order));
        expr_dump({"expr", h.depth + 1}, unary->expr);
        break;
    }

    case Ast_Expr_Binary: {
        Binary_Expr *binary = (Binary_Expr *)ast_expr;
        print("{} binary-expr [op: '{:s}']\n", h, binary->op.expr);
        expr_dump({"prev", h.depth + 1}, binary->prev);
        expr_dump({"post", h.depth + 1}, binary->post);
        entity_dump({"type", h.depth + 1}, binary->type);
        break;
    }

    case Ast_Expr_Nested: {
        Nested_Expr *nested = (Nested_Expr *)ast_expr;
        print("{} nested-expr\n", h);
        expr_dump({"expr", h.depth + 1}, nested->expr);
        break;
    }

    case Ast_Expr_Scope: {
        Scope_Expr *scope = (Scope_Expr *)ast_expr;
        print("{} scope-expr\n", h);
        frame_dump({"frame", h.depth + 1}, scope->frame);

        if (!scope->compound or scope->compound->empty())
        {
            print("(empty-scope)\n");
            break;
        }

        for (Ast_Expr *expr : *scope->compound)
        {
            expr_dump({">", h.depth + 1}, expr);
        }
        break;
    }

    case Ast_Expr_Return: {
        Return_Expr *return_expr = (Return_Expr *)ast_expr;
        print("{} return-expr\n", h);
        expr_dump({"expr", h.depth + 1}, return_expr->expr);
        break;
    }

    case Ast_Expr_Id: {
        Id_Expr *id = (Id_Expr *)ast_expr;
        print("{} id-expr\n", h);
        entity_dump({"entity", h.depth + 1}, id->entity);
        break;
    }

    case Ast_Expr_Var: {
        Var_Expr *var = (Var_Expr *)ast_expr;
        print("{} var-expr [name: '{:s}', op: '{:s}']\n", h, var->name.expr, var->op.expr);
        entity_dump({"var", h.depth + 1}, var->var);
        expr_dump({"expr", h.depth + 1}, var->expr);
        expr_dump({"next", h.depth + 1}, var->next);
        break;
    }

    case Ast_Expr_Char: {
        Char_Expr *char_expr = (Char_Expr *)ast_expr;
        print("{} char-expr [data: '{:c}']\n", h, char_expr->data);
        break;
    }

    case Ast_Expr_Str: {
        Str_Expr *str_expr = (Str_Expr *)ast_expr;
        print("{} str-expr [data: '{:s}']\n", h, str_expr->data);
        break;
    }

    case Ast_Expr_Int: {
        Int_Expr *int_expr = (Int_Expr *)ast_expr;
        print("{} int-expr [data: '{:d}', size: {:d}]\n", h, int_expr->data, int_expr->size);
        break;
    }

    case Ast_Expr_Float: {
        Float_Expr *float_expr = (Float_Expr *)ast_expr;
        print("{} float-expr [data: '{:f}', size: {:d}]\n", h, float_expr->data, float_expr->size);
        break;
    }

    case Ast_Expr_Signature: {
        Signature_Expr *signature = (Signature_Expr *)ast_expr;
        print("{} signature-expr\n", h);
        expr_dump({"params", h.depth + 1}, signature->params);
        entity_dump({"type", h.depth + 1}, signature->type);
        frame_dump({"frame", h.depth + 1}, signature->frame);
        break;
    }

    case Ast_Expr_Function: {
        Function_Expr *function = (Function_Expr *)ast_expr;
        print("{} function-expr\n", h);
        entity_dump({"function", h.depth + 1}, function->function);
        expr_dump({"scope", h.depth + 1}, function->scope);
        break;
    }

    case Ast_Expr_Argument: {
        Argument_Expr *argument = (Argument_Expr *)ast_expr;
        print("{} argument-expr\n", h);
        expr_dump({"expr", h.depth + 1}, argument->expr);
        expr_dump({"next", h.depth + 1}, argument->next);
        break;
    }

    case Ast_Expr_Invoke: {
        Invoke_Expr *invoke = (Invoke_Expr *)ast_expr;
        print("{} invoke-expr\n", h);
        expr_dump({"args", h.depth + 1}, invoke->args);
        entity_dump({"function", h.depth + 1}, invoke->function);
        break;
    }

    case Ast_Expr_If: {
        If_Expr *if_expr = (If_Expr *)ast_expr;
        print("{} if-expr\n", h);
        expr_dump({"condition", h.depth + 1}, if_expr->condition);
        expr_dump({"scope-if", h.depth + 1}, if_expr->scope_if);
        expr_dump({"scope-else", h.depth + 1}, if_expr->scope_else);
        break;
    }

    case Ast_Expr_For: {
        For_Expr *for_expr = (For_Expr *)ast_expr;
        print("{} for-expr\n", h);
        expr_dump({"start", h.depth + 1}, for_expr->start);
        expr_dump({"condition", h.depth + 1}, for_expr->condition);
        expr_dump({"iteration", h.depth + 1}, for_expr->iteration);
        expr_dump({"scope", h.depth + 1}, for_expr->scope);
        frame_dump({"frame", h.depth + 1}, for_expr->frame);
        break;
    }

    case Ast_Expr_For_While: {
        For_While_Expr *for_expr = (For_While_Expr *)ast_expr;
        print("{} for-while-expr\n", h);
        expr_dump({"condition", h.depth + 1}, for_expr->condition);
        expr_dump({"scope", h.depth + 1}, for_expr->scope);
        break;
    }

    default: {
        print("{} TODO! Implement expression '{:s}' for expr_dump()'\n", h, ast_expr_kind_name(ast_expr->kind()));
        break;
    }
    }
}

void Ast_Dump::entity_dump(Ast_Dump_Header h, Ast_Entity *ast_entity)
{
    if (!ast_entity)
    {
        print("{} (none)\n", h);
        return;
    }

    switch (ast_entity->kind())
    {
    case Ast_Entity_Var: {
        Var *var = (Var *)ast_entity;
        print("{} var [name: '{}', begin: {}, end: {}]\n", h, var->name, var->begin, var->end);
        entity_dump({"type", h.depth + 1}, var->type);
        break;
    }

    case Ast_Entity_Function: {
        Function *function = (Function *)ast_entity;
        print("{} function '{}'\n", h, function->repr());
        expr_dump({"params", h.depth + 1}, function->params);
        entity_dump({"type", h.depth + 1}, function->type);
        break;
    }

    case Ast_Entity_Void: {
        print("{} void\n", h);
        break;
    }

    case Ast_Entity_Atom: {
        Atom_Type *atom = (Atom_Type *)ast_entity;
        std::string_view desc = atom_desc_name(atom->desc);
        print("{} atom [name: '{}', desc: '{:s}', size: {:d}]\n", h, atom->name, desc, atom->size);
        break;
    }

    default: {
        print("{} TODO! Implement entity '{:s}' for entity_dump()'\n", h, ast_entity_kind_name(ast_entity->kind()));
        break;
    }
    }
}

} // namespace bee
