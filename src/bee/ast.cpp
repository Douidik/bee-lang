#include "ast.hpp"

namespace bee
{

Ast::~Ast()
{
    for (Frame *frame : frames)
        delete frame;
}

Compound_Expr *Ast::push_compound(Compound_Expr compound)
{
    return &compounds.emplace_back(compound);
}

Frame *Ast::push_frame(Frame *f)
{
    f->owner = frame;
    frame = f;

    if (frame->owner != NULL)
        frame->depth = frame->owner->depth + 1;
    else
        frame->depth = 0;
    
    if (frame->depth >= Ast_Frame_Limit)
        throw errorf("frame depth limit exceeded");
    
    frames.insert(f);
    return f;
}

Frame *Ast::pop_frame()
{
    Frame *pop = frame;
    frame = frame->owner;
    return pop;
}

} // namespace bee
