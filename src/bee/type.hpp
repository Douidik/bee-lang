#ifndef BEE_TYPE_HPP
#define BEE_TYPE_HPP

#include "entity.hpp"

namespace bee
{
struct Def_Expr;
struct Ast_Entity;

struct Void_Type : Ast_Entity_Impl<Ast_Entity_Void>
{
};

struct Signature : Ast_Entity_Impl<Ast_Entity_Signature>
{
    Def_Expr *params;
    Ast_Entity *type;

    std::string make_name() const;
};

enum Atom_Desc : u32
{
    Atom_Raw = 0,
    Atom_Signed = 1 << 0,
    Atom_Float = 1 << 1,
};

struct Atom_Type : Ast_Entity_Impl<Ast_Entity_Atom>
{
    Atom_Desc desc;
    u32 size;
};

struct Struct_Type : Ast_Entity_Impl<Ast_Entity_Struct>
{
};

struct Enum_Type : Ast_Entity_Impl<Ast_Entity_Enum>
{
};

constexpr std::string_view atom_desc_name(Atom_Desc atom_desc)
{
    switch (atom_desc)
    {
    case Atom_Raw:
        return "Atom_Raw";
    case Atom_Signed:
        return "Atom_Signed";
    case Atom_Float:
        return "Atom_Float";
    default:
        return "?";
    }
}

} // namespace bee

#endif
