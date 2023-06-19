#include "expr.hpp"
#include "var.hpp"

namespace bee
{

std::string Signature_Expr::name() const
{
    Standard_Stream stream = {};

    stream.std_print("(");
    for (Var_Expr *def = params; def != NULL; def = def->next)
    {
        stream.std_print("{:s}: {:s}", def->name.expr, def->var->type->name);

        if (def->next != NULL)
        {
            stream.std_print(", ");
        }
    }
    stream.std_print(")");

    if (type->kind() != Ast_Entity_Void)
    {
        stream.std_print(" -> {:s}", type->name);
    }

    return std::string{stream.str()};
}

} // namespace bee
