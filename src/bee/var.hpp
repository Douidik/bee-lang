#ifndef BEE_VAR_HPP
#define BEE_VAR_HPP

#include "entity.hpp"

namespace bee
{

struct Var : Ast_Entity_Impl<Ast_Entity_Var>
{
    Ast_Entity *type;
};

} // namespace bee

#endif
