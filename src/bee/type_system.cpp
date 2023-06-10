#include "type_system.hpp"

namespace bee
{

void Type_System::def_atom_types(Ast *ast)
{
    auto type = [ast](std::string_view name, Atom_Type type) -> Ast_Entity * {
        return ast->entity_push<Atom_Type>(name, type);
    };

    f16_type = type("f16", Atom_Type{Atom_Float, 2});
    f32_type = type("f32", Atom_Type{Atom_Float, 4});
    f64_type = type("f64", Atom_Type{Atom_Float, 8});
    s8_type = type("s8", Atom_Type{Atom_Signed, 1});
    s16_type = type("s16", Atom_Type{Atom_Signed, 2});
    s32_type = type("s32", Atom_Type{Atom_Signed, 4});
    s64_type = type("s64", Atom_Type{Atom_Signed, 8});
    u8_type = type("u8", Atom_Type{Atom_Raw, 1});
    u16_type = type("u16", Atom_Type{Atom_Raw, 2});
    u32_type = type("u32", Atom_Type{Atom_Raw, 4});
    u64_type = type("u64", Atom_Type{Atom_Raw, 8});
    char_type = type("char", Atom_Type{Atom_Signed, 1});
    ssize_type = type("ssize", Atom_Type{Atom_Signed, sizeof(usize)});
    usize_type = type("usize", Atom_Type{Atom_Raw, sizeof(usize)});
    void_type = type("void", Atom_Type{});
}

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
    case Ast_Entity_Proc:
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

Ast_Entity *Type_System::type_expr(Ast_Expr *ast_expr)
{
    switch (ast_expr->kind)
    {
    case Ast_Expr_Unary:
        return type_expr(ast_expr->unary_expr.expr);

    case Ast_Expr_Binary:
        return type_expr(ast_expr->binary_expr.prev);

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

    case Ast_Expr_Int: {
        u32 size = ast_expr->int_expr.size;
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
        throw errorf("cannot create integer type of size '{:d}'", size);
    }

    case Ast_Expr_Float: {
        u32 size = ast_expr->float_expr.size;
        switch (size)
        {
        case 2:
            return f16_type;
        case 4:
            return f32_type;
        case 8:
            return f64_type;
        }
        throw errorf("cannot create float type of size '{:d}'", size);
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

} // namespace bee