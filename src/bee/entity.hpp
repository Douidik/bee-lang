#ifndef BEE_ENTITY_HPP
#define BEE_ENTITY_HPP

#include "type.hpp"
#include "var.hpp"

namespace bee
{

enum Ast_Entity_Kind : u32
{
    Ast_Entity_None = 0,
    Ast_Entity_Var = 1 << 0,
    Ast_Entity_Proc = 1 << 2,

    Ast_Entity_Void = 1 << 3,
    Ast_Entity_Atom = 1 << 4,
    Ast_Entity_Struct = 1 << 5,
    Ast_Entity_Enum = 1 << 6,
    Ast_Entity_Type = Ast_Entity_Void | Ast_Entity_Atom | Ast_Entity_Struct | Ast_Entity_Enum,
};

struct Ast_Entity
{
    Ast_Entity_Kind kind;
    union {
        Var var;
        Void_Type void_type;
        Atom_Type atom_type;
        Struct_Type struct_type;
        Enum_Type enum_type;
    };

    Ast_Entity() : kind{Ast_Entity_None} {}
    Ast_Entity(Var var) : kind{Ast_Entity_Var}, var{var} {}
    Ast_Entity(Void_Type void_type) : kind{Ast_Entity_Void}, void_type{void_type} {}
    Ast_Entity(Atom_Type atom_type) : kind{Ast_Entity_Atom}, atom_type{atom_type} {}
    Ast_Entity(Struct_Type struct_type) : kind{Ast_Entity_Atom}, struct_type{struct_type} {}
    Ast_Entity(Enum_Type enum_type) : kind{Ast_Entity_Atom}, enum_type{enum_type} {}
};

constexpr std::string_view ast_entity_kind_name(Ast_Entity_Kind kind)
{
    switch (kind)
    {
    case Ast_Entity_None:
        return "None";
    case Ast_Entity_Var:
        return "Var";
    case Ast_Entity_Proc:
        return "Proc";
    case Ast_Entity_Void:
        return "Void";
    case Ast_Entity_Atom:
        return "Atom";
    case Ast_Entity_Struct:
        return "Struct";
    case Ast_Entity_Enum:
        return "Enum";
    default:
        return "?";
    }
}

} // namespace bee

#endif
