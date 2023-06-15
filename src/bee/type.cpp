#include "type.hpp"
#include "ast.hpp"
#include "entity.hpp"
#include "expr.hpp"
#include "var.hpp"

namespace bee
{

std::string Signature_Type::make_name() const
{
    Standard_Stream stream = {};

    stream.std_print("(");
    for (Ast_Expr *expr = params; expr != NULL; expr = expr->def_expr.next)
    {
        Def_Expr *def = &expr->def_expr;
        Var *var = &expr->def_expr.entity->var;
        stream.std_print("{:s}: {:s}", def->name.expr, var->type->name);

        if (def->next != NULL)
        {
            stream.std_print(", ");
        }
    }
    stream.std_print(")");

    if (type->kind != Ast_Entity_Void)
    {
        stream.std_print(" -> {:s}", type->name);
    }

    return std::string{stream.str()};
}

} // namespace bee
