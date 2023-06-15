#include "type_system.hpp"

namespace bee
{

u32 Type_System::type_cast(Ast_Entity *from, Ast_Entity *into)
{
    // TODO! Enum type_cast()

    if (from->kind & Ast_Entity_Var)
        return type_cast(from->var.type, into);
    if (into->kind & Ast_Entity_Var)
        return type_cast(from, into->var.type);
    if (from->kind != into->kind)
        return Type_Cast_Error;

    switch (from->kind)
    {
    case Ast_Entity_Signature:
        // TODO! Each of the arguments and return types are the Type_Cast_Same
        return Type_Cast_Same;

    case Ast_Entity_Void:
        return Type_Cast_Same;

    case Ast_Entity_Atom: {
        u32 cast = 0;

        if (from->atom_type.desc != into->atom_type.desc)
            cast |= Type_Cast_Transmuted;
        if (from->atom_type.size > into->atom_type.size)
            cast |= Type_Cast_Inferred;
        if (from->atom_type.size < into->atom_type.size)
            cast |= Type_Cast_Narrowed;

        return cast != 0 ? cast : Type_Cast_Same;
    }

    case Ast_Entity_Struct: {
        return Type_Cast_Same;
    }

    case Ast_Entity_Enum: {
        // TODO! enum key type
        return Type_Cast_Same;
    }

    default:
        return Type_Cast_Error;
    }
}

u32 Type_System::type_size(Ast_Entity *ast_entity)
{
    switch (ast_entity->kind)
    {
    case Ast_Entity_Var:
        return type_size(ast_entity->var.type);

    case Ast_Entity_Atom:
        return ast_entity->atom_type.size;

    default:
        return 0;
    }
}

Ast_Entity *Type_System::type_expr(Ast_Expr *ast_expr)
{
    switch (ast_expr->kind)
    {
    case Ast_Expr_Unary:
        return type_expr(ast_expr->unary_expr.expr);

    case Ast_Expr_Binary:
        return ast_expr->binary_expr.type;

    case Ast_Expr_Nested:
        return type_expr(ast_expr->nested_expr.expr);

    case Ast_Expr_Id:
        return type_entity(ast_expr->id_expr.entity);

    case Ast_Expr_Def:
        return type_entity(ast_expr->def_expr.entity);

    case Ast_Expr_Char:
        return char_type;

    case Ast_Expr_Str:
        throw errorf("TODO! Ast_Expr_Str type deduction must implement pointers/arrays");

    case Ast_Expr_Proc:
        return type_entity(ast_expr->proc_expr.signature);

    case Ast_Expr_Invoke: {
        // Invoke_Expr *invoke = &ast_expr->invoke_expr;
        // Proc_Expr *proc = &invoke->proc->proc_expr;
        // Signature_Type *signature = &proc->signature->signature_type;
        Ast_Entity *signature = type_expr(ast_expr->invoke_expr.proc);
        return signature->signature_type.type;
    }

    case Ast_Expr_Int: {
        u32 size = ast_expr->int_expr.size;
        Ast_Entity *type = compose_atom(Atom_Signed, size);

        if (!type)
            throw errorf("cannot create integer type of size '{:d}'", size);
        return type;
    }

    case Ast_Expr_Float: {
        u32 size = ast_expr->int_expr.size;
        Ast_Entity *type = compose_atom(Atom_Float, size);

        if (!type)
            throw errorf("cannot create float type of size '{:d}'", size);
        return type;
    }

    default:
        return void_type;
    }
}

Ast_Entity *Type_System::type_entity(Ast_Entity *ast_entity)
{
    if (ast_entity->kind & Ast_Entity_Type)
    {
        return ast_entity;
    }

    if (ast_entity->kind & Ast_Entity_Var)
    {
        return ast_entity->var.type;
    }

    return void_type;
}

void Type_System::type_std_def(Ast *ast)
{
    auto atom = [ast](std::string_view name, Atom_Desc desc, u32 size) -> Ast_Entity * {
        return ast->frame->push<Atom_Type>(name, Atom_Type{desc, size});
    };

    f16_type = atom("f16", Atom_Float, 2);
    f32_type = atom("f32", Atom_Float, 4);
    f64_type = atom("f64", Atom_Float, 8);
    s8_type = atom("s8", Atom_Signed, 1);
    s16_type = atom("s16", Atom_Signed, 2);
    s32_type = atom("s32", Atom_Signed, 4);
    s64_type = atom("s64", Atom_Signed, 8);
    u8_type = atom("u8", Atom_Raw, 1);
    u16_type = atom("u16", Atom_Raw, 2);
    u32_type = atom("u32", Atom_Raw, 4);
    u64_type = atom("u64", Atom_Raw, 8);
    char_type = atom("char", Atom_Signed, 1);
    ssize_type = atom("ssize", Atom_Signed, sizeof(usize));
    usize_type = atom("usize", Atom_Raw, sizeof(usize));
    void_type = ast->frame->push<Void_Type>("void");
}

Ast_Entity *Type_System::compose_atom(u32 desc, u32 size)
{
    if (desc & Atom_Float)
    {
        switch (size)
        {
        case 2:
            return f16_type;
        case 4:
            return f32_type;
        case 8:
            return f64_type;
        }
    }

    if (desc & Atom_Signed)
    {
        switch (size)
        {
        case 1:
            return s8_type;
        case 2:
            return s16_type;
        case 4:
            return s32_type;
        case 8:
            return s64_type;
        }
    }

    if (desc & Atom_Raw)
    {
        switch (size)
        {
        case 1:
            return u8_type;
        case 2:
            return u16_type;
        case 4:
            return u32_type;
        case 8:
            return u64_type;
        }
    }

    return NULL;
}

} // namespace bee
