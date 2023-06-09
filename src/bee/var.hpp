#ifndef BEE_VAR_HPP
#define BEE_VAR_HPP

#include "entity.hpp"
#include "type.hpp"

namespace bee
{

struct Var : Ast_Entity
{
    Type type;

    Var(Type type) : type{type} {}
};

} // namespace bee

#endif
