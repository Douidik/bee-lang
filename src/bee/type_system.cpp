#include "type_system.hpp"
#include "function.hpp"
#include "type.hpp"
#include "var.hpp"

namespace bee
{

u32 Type_System::cast_type(Ast_Entity *from, Ast_Entity *into)
{
    // TODO! Enum type_cast()

    if (from->kind() & Ast_Entity_Var)
        return cast_type(((Var *)from)->type, into);
    if (into->kind() & Ast_Entity_Var)
        return cast_type(from, ((Var *)into)->type);
    if (from->kind() != into->kind())
        return Type_Cast_Error;

    switch (from->kind())
    {
    case Ast_Entity_Function: {
        Function *from_function = (Function *)from;
        Function *into_function = (Function *)into;

        if (cast_type(from_function->type, into_function->type) != Type_Cast_Same)
            return Type_Cast_Error;

        for (Var_Expr *from_def = from_function->params, *into_def = into_function->params;
             from_def != NULL or into_def != NULL; from_def = from_def->next, into_def = into_def->next)
        {
            if (from_def != NULL ^ into_def != NULL)
                return Type_Cast_Error;

            Ast_Entity *from_type = expr_type(from_def->expr);
            Ast_Entity *into_type = expr_type(into_def->expr);

            if (cast_type(from_type, into_type) != Type_Cast_Same)
                return Type_Cast_Error;
        }

        return Type_Cast_Same;
    }

    case Ast_Entity_Void:
        return Type_Cast_Same;

    case Ast_Entity_Atom: {
        u32 cast = 0;
        Atom_Type *from_atom = (Atom_Type *)from;
        Atom_Type *into_atom = (Atom_Type *)into;

        // if ((from_atom->desc & Atom_Float) != (into_atom->desc & Atom_Float))
        if ((from_atom->desc ^ into_atom->desc) & Atom_Float)
            cast |= Type_Cast_Transmuted;
        if (from_atom->max() > into_atom->max())
            cast |= Type_Cast_Inferred;
        if (from_atom->max() < into_atom->max())
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

u32 Type_System::size_type(Ast_Entity *ast_entity)
{
    switch (ast_entity->kind())
    {
    case Ast_Entity_Var:
        return size_type(((Var *)ast_entity)->type);

    case Ast_Entity_Atom:
        return ((Atom_Type *)ast_entity)->size;

    default:
        return 0;
    }
}

Ast_Entity *Type_System::expr_type(Ast_Expr *ast_expr)
{
    switch (ast_expr->kind())
    {
    case Ast_Expr_Unary:
        return expr_type(((Unary_Expr *)ast_expr)->expr);

    case Ast_Expr_Binary:
        return ((Binary_Expr *)ast_expr)->type;

    case Ast_Expr_Nested:
        return expr_type(((Nested_Expr *)ast_expr)->expr);

    case Ast_Expr_Id:
        return entity_type(((Id_Expr *)ast_expr)->entity);

    case Ast_Expr_Var:
        return entity_type(((Var_Expr *)ast_expr)->var);

    case Ast_Expr_Char:
        return char_type;

    case Ast_Expr_Str:
        throw errorf("TODO! Ast_Expr_Str type deduction must implement pointers/arrays");

    case Ast_Expr_Function:
        return entity_type(((Function_Expr *)ast_expr)->function);

    case Ast_Expr_Invoke: {
        Invoke_Expr *invoke = (Invoke_Expr *)ast_expr;
        return invoke->function->type;
    }

    case Ast_Expr_Int: {
        Int_Expr *expr = (Int_Expr *)ast_expr;
        Ast_Entity *type = compose_atom(Atom_Signed, expr->size);

        if (!type)
            throw errorf("cannot create integer type of size '{:d}'", expr->size);
        return type;
    }

    case Ast_Expr_Float: {
        Float_Expr *expr = (Float_Expr *)ast_expr;
        Ast_Entity *type = compose_atom(Atom_Float, expr->size);

        if (!type)
            throw errorf("cannot create float type of size '{:d}'", expr->size);
        return type;
    }

    default:
        return void_type;
    }
}

Ast_Entity *Type_System::entity_type(Ast_Entity *ast_entity)
{

    if (!ast_entity)
    {
        return NULL;
    }
    if (ast_entity->kind() & Ast_Entity_Type)
    {
        return ast_entity;
    }
    if (ast_entity->kind() & Ast_Entity_Var)
    {
        return ((Var *)ast_entity)->type;
    }

    return void_type;
}

void Type_System::std_types(Ast *ast)
{
    auto atom = [ast](std::string_view name, Atom_Desc desc, u32 size) -> Ast_Entity * {
        Atom_Type *atom = new Atom_Type;
        atom->name = name;
        atom->desc = desc;
        atom->size = size;
        return ast->frame->push(atom);
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
    bool_type = atom("bool", Atom_Raw, 1);
    ssize_type = atom("ssize", Atom_Signed, sizeof(usize));
    usize_type = atom("usize", Atom_Raw, sizeof(usize));

    void_type = new Void_Type;
    void_type->name = "void";
    ast->frame->push(void_type);
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

    else if (desc & Atom_Signed)
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

    else
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
