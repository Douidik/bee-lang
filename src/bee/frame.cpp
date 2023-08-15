#include "frame.hpp"

namespace bee
{

Frame::~Frame()
{
    for (auto &[name, entity] : defs)
        delete entity;
}

Ast_Entity *Frame::find_def(std::string_view name)
{
    if (!defs.contains(name))
    {
        if (owner != NULL)
            return owner->find_def(name);
        return NULL;
    }
    return defs.at(name);
}

u8 *Frame::find_ref(Ast_Entity *entity)
{
    if (!refs.contains(entity))
    {
        if (owner != NULL)
            return owner->find_ref(entity);
        return NULL;
    }
    return refs.at(entity);
}

u8 *Frame::push_ref(Ast_Entity *entity, u8 *ref)
{
    return refs[entity] = ref;
}

Scope_Expr *Frame::push_function(Function *function, Scope_Expr *scope)
{
    if (refs.contains(function))
    {
        throw errorf("redefinition of function '{:s}'", function->name);
    }
    return binds[function] = scope;
}

Scope_Expr *Frame::find_function(Function *function)
{
    if (!binds.contains(function))
    {
        if (owner != NULL)
            return owner->find_function(function);
        return NULL;
    }
    return binds.at(function);
}

} // namespace bee
