#include "function.hpp"
#include "var.hpp"

namespace bee
{

std::string Function::repr() const
{
    Stream stream = {};

    stream.print("{:s}(", name);
    for (Var_Expr *def = params; def != NULL; def = def->next)
    {
        stream.print("{:s}: {:s}", def->name.expr, def->var->type->name);

        if (def->next != NULL)
        {
            stream.print(", ");
        }
    }
    stream.print(")");

    if (type->kind() != Ast_Entity_Void)
    {
        stream.print(" -> {:s}", type->name);
    }

    return std::string{stream.str()};
}

} // namespace bee
