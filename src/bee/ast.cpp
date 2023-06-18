#include "ast.hpp"

namespace bee
{

Ast_Frame::~Ast_Frame()
{
    for (auto &[name, entity] : defs)
        delete entity;
}

Ast_Entity *Ast_Frame::find(std::string_view name)
{
    if (auto match = defs.find(name); match != defs.end())
        return match->second;
    return owner != NULL ? owner->find(name) : NULL;
}

Ast_Entity *Ast_Frame::push(Ast_Entity *entity)
{
    return defs[entity->name] = entity;
}

Compound_Expr *Ast::compound_push(const Compound_Expr &&compound)
{
    return &compounds.push(std::move(compound));
}

Ast_Frame *Ast::stack_push()
{
    return frame = &stack.push(Ast_Frame{.owner = frame});
}

Ast_Frame *Ast::stack_pop()
{
    stack.pop();
    return (frame = frame->owner);
}

} // namespace bee
