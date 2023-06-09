#include "ast.hpp"

namespace bee
{

Ast_Entity *Frame::find(std::string_view name)
{
    if (auto match = defs.find(name); match != defs.end())
        return match->second;
    return owner != NULL ? owner->find(name) : NULL;
}

void Frame::free()
{
    for (auto &[name, entity] : defs)
    {
        delete entity;
    }
}

void Ast::free()
{
    for (Ast_Expr *expr : exprs)
    {
        delete expr;
    }
    for (usize i = 0; i < stack.size; i++)
    {
        stack.buffer[i].free();
        stack.pop();
    }
}

Ast_Expr *Ast::expr_push(Ast_Expr *expr)
{
    return exprs.emplace_back(expr);
}

Frame *Ast::stack_push()
{
    return (frame = &stack.push(Frame{.owner = frame}));
}

Frame *Ast::stack_pop()
{
    stack.back()->free();
    return (frame = stack.pop());
}

} // namespace bee
