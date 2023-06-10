#include "ast.hpp"

namespace bee
{

Ast_Entity *Ast_Frame::find(std::string_view name)
{
    if (auto match = defs.find(name); match != defs.end())
        return &match->second;
    return owner != NULL ? owner->find(name) : NULL;
}

Ast_Frame *Ast::stack_push()
{
    return frame = &stack.push(Ast_Frame{.owner = frame});
}

Ast_Frame *Ast::stack_pop()
{
    return frame = stack.pop();
}

} // namespace bee
