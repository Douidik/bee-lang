#include "ast.hpp"

namespace bee
{

Ast_Frame::~Ast_Frame()
{
    for (auto &[name, entity] : defs)
        delete entity;
}

Ast::~Ast()
{
    for (Ast_Frame *frame : frames)
        delete frame;
}

Ast_Entity *Ast_Frame::find(std::string_view name)
{
    if (auto match = defs.find(name); match != defs.end())
        return match->second;
    return owner != NULL ? owner->find(name) : NULL;
}

Compound_Expr *Ast::push_compound(Compound_Expr compound)
{
    return &compounds.emplace_back(compound);
}

Ast_Frame *Ast::push_frame(Ast_Frame *f)
{
    f->owner = frame;
    frame = f;
    frames.insert(f);
    return f;
}
    
Ast_Frame *Ast::pop_frame()
{
    Ast_Frame *pop = frame;
    frame = frame->owner;
    return pop;
}

} // namespace bee
