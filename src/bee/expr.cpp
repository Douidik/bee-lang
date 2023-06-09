#include "expr.hpp"
#include "escape_sequence.hpp"

namespace bee
{

Ast_Dump_Stream &Unary_Expr::ast_dump(Ast_Dump_Stream &stream, s32 depth) const
{
    stream.print(depth, "Unary_Expr (op: '{:s}', order: '{:s}')\n", op.expr, order_expr_name(order));
    return expr->ast_dump(stream, depth + 1);
}

Type Unary_Expr::type() const
{
    return expr->type();
}

Ast_Dump_Stream &Binary_Expr::ast_dump(Ast_Dump_Stream &stream, s32 depth) const
{
    stream.print(depth, "Binary_Expr (op: '{:s}')\n", op.expr);
    prev->ast_dump(stream, depth + 1);
    post->ast_dump(stream, depth + 1);
    return stream;
}

Type Binary_Expr::type() const
{
    return prev->type();
}

Ast_Dump_Stream &Nested_Expr::ast_dump(Ast_Dump_Stream &stream, s32 depth) const
{
    return stream;
}

Type Nested_Expr::type() const
{
    return expr->type();
}

Ast_Dump_Stream &Scope_Expr::ast_dump(Ast_Dump_Stream &stream, s32 depth) const
{
    stream.print(depth, "Scope_Expr\n");
    for (Ast_Expr *expr : body)
    {
        expr->ast_dump(stream, depth + 1);
    }
    return stream;
}

Ast_Dump_Stream &Id_Expr::ast_dump(Ast_Dump_Stream &stream, s32 depth) const
{
    stream.print(depth, "Id_Expr\n");
    return var->ast_dump(stream, depth + 1);
}

Type Id_Expr::type() const
{
    return var->type;
}

Ast_Dump_Stream &Def_Expr::ast_dump(Ast_Dump_Stream &stream, s32 depth) const
{
    stream.print(depth, "Def_Expr (op: '{:s}')\n", op.expr);
    entity->ast_dump(stream, depth + 1);
    expr->ast_dump(stream, depth + 1);
    return stream;
}

Ast_Dump_Stream &Char_Expr::ast_dump(Ast_Dump_Stream &stream, s32 depth) const
{
    return stream.print(depth, "Char_Expr (data: '{:s}')\n", escape_sequence(data));
}

Type Char_Expr::type() const
{
    return Type::make_atom(Atom_Signed, 1);
}

Ast_Dump_Stream &Str_Expr::ast_dump(Ast_Dump_Stream &stream, s32 depth) const
{
    return stream.print(depth, "Str_Expr (data: '{:s}')\n", escape_string(data));
}

Ast_Dump_Stream &Int_Expr::ast_dump(Ast_Dump_Stream &stream, s32 depth) const
{
    return stream.print(depth, "Int_Expr (data: '{:d}', size '{:d}')\n", data, size);
}

Type Int_Expr::type() const
{
    return Type::make_atom(Atom_Signed, size);
}

Ast_Dump_Stream &Float_Expr::ast_dump(Ast_Dump_Stream &stream, s32 depth) const
{
    return stream.print(depth, "Float_Expr (data: '{:f}', size '{:d}')\n", data, size);
}

Ast_Dump_Stream &Proc_Expr::ast_dump(Ast_Dump_Stream &stream, s32 depth) const
{
    stream.print(depth, "Proc_Expr\n");

    for (Def_Expr *argument : args)
    {
        argument->ast_dump(stream, depth + 1);
    }
    return scope->ast_dump(stream, depth + 1);
}

Ast_Dump_Stream &Invoke_Expr::ast_dump(Ast_Dump_Stream &stream, s32 depth) const
{
    stream.print(depth, "Invoke_Expr\n");
    for (Ast_Expr *argument : args)
    {
        argument->ast_dump(stream, depth + 1);
    }
    return stream;
}

} // namespace bee
