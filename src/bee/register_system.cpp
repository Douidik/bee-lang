#include "register_system.hpp"
#include "ast.hpp"
#include "expr.hpp"
#include "frame.hpp"
#include "var.hpp"
#include <ranges>

// https://en.wikipedia.org/wiki/Register_allocation#Graph-coloring_allocation

namespace bee
{

bool Register_Allocator::Sort_By_Start::operator()(const Var *v, const Var *w) const
{
    return v->begin < w->begin;
}

bool Register_Allocator::Sort_By_End::operator()(const Var *v, const Var *w) const
{
    return v->end < w->end;
}

Register_Allocator::Register_Allocator(Function_Expr *function, std::span<Register> regs, Type_System &type_system) :
    function{function},
    regs{regs},
    type_system{type_system}
{
}

void Register_Allocator::allocate()
{
    parse_intervals(function->scope);
    for (Register &reg : regs)
    {
        regs_free.insert(&reg);
    }

    for (const auto &v : vars)
    {
        expire_vars(v);
        if (regs_free.empty())
        {
            spill_var(v);
        }
        else
        {
            v->reg = regs_free.extract(regs_free.begin()).value();
            active.insert(v);
        }
    }
}

void Register_Allocator::expire_vars(Var *v)
{
    for (auto &w : active)
    {
        if (v->end >= w->begin)
            return;
        active.erase(w);
        regs_free.insert(w->reg);
    }
}

void Register_Allocator::spill_var(Var *v)
{
    Var *spill = *active.rbegin();

    if (spill->end > v->end)
    {
        v->reg = spill->reg;
        spill->reg = NULL;
        active.erase(spill);
        active.insert(v);
    }
    else
    {
        v->reg = NULL;
    }
}

void Register_Allocator::parse_intervals(Ast_Expr *expr)
{
    if (!expr)
        return;
    time++;

    const auto report_var = [&](Var *var) {
        vars.insert(var);
        if (var->begin == (u32)-1)
            var->begin = time;
        var->end = time;
    };

    switch (expr->kind())
    {
    case Ast_Expr_Id: {
        Id_Expr *id = (Id_Expr *)expr;
        if (id->entity != NULL and id->entity->kind() & Ast_Entity_Var)
        {
            report_var((Var *)id->entity);
        }
        return;
    }

    case Ast_Expr_Var: {
        Var_Expr *def = (Var_Expr *)expr;
        if (def->expr != NULL)
        {
            parse_intervals(def->expr);
            report_var(def->var);
        }
        return;
    }

    case Ast_Expr_Nested:
        return parse_intervals(((Nested_Expr *)expr)->expr);

    case Ast_Expr_Unary:
        return parse_intervals(((Unary_Expr *)expr)->expr);

    case Ast_Expr_Binary: {
        Binary_Expr *binary = (Binary_Expr *)expr;
        parse_intervals(binary->prev);
        parse_intervals(binary->post);
        return;
    }

    case Ast_Expr_Scope: {
        Scope_Expr *scope = (Scope_Expr *)expr;

        for (Ast_Expr *scope_expr : *scope->compound)
        {
            parse_intervals(scope_expr);
        }
        return;
    }

    case Ast_Expr_Return:
        return parse_intervals(((Return_Expr *)expr)->expr);

    case Ast_Expr_Invoke:
        return parse_intervals(((Invoke_Expr *)expr)->args);

    case Ast_Expr_Argument: {
        Argument_Expr *argument = (Argument_Expr *)expr;
        parse_intervals(argument->expr);
        parse_intervals(argument->next);
        return;
    }

    case Ast_Expr_If: {
        If_Expr *if_expr = (If_Expr *)expr;
        parse_intervals(if_expr->condition);
        parse_intervals(if_expr->scope_if);
        parse_intervals(if_expr->scope_else);
        return;
    }

    case Ast_Expr_For: {
        For_Expr *for_expr = (For_Expr *)expr;
        parse_intervals(for_expr->start);
        parse_intervals(for_expr->condition);
        parse_intervals(for_expr->iteration);
        parse_intervals(for_expr->scope);
        return;
    }

    case Ast_Expr_For_While: {
        For_While_Expr *for_expr = (For_While_Expr *)expr;
        parse_intervals(for_expr->condition);
        parse_intervals(for_expr->scope);
        return;
    }

    default:
        return;
    }
}

} // namespace bee
