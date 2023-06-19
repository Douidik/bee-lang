#ifndef BEE_FUNCTION_HPP
#define BEE_FUNCTION_HPP

#include "entity.hpp"
#include "expr.hpp"

namespace bee
{

struct Function : Ast_Entity_Impl<Ast_Entity_Function>
{
    Var_Expr *params;
    Ast_Entity *type;
    Function *owner;

    std::string repr() const;
};

} // namespace bee

#endif
