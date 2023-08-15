#ifndef BEE_VAR_HPP
#define BEE_VAR_HPP

#include "entity.hpp"
#include "register.hpp"
#include <vector>

namespace bee
{

struct Var : Ast_Entity_Impl<Ast_Entity_Var>
{
    Ast_Entity *type;
    u32 begin;
    u32 end;
};

} // namespace bee

#endif
