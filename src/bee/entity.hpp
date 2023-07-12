#ifndef BEE_ENTITY_HPP
#define BEE_ENTITY_HPP

#include "core.hpp"
#include "bitset.hpp"
#include "error.hpp"
#include <string>

namespace bee
{

enum Ast_Entity_Kind : u32
{
    Ast_Entity_None = 0,
    Ast_Entity_Var = bitset(0),
    Ast_Entity_Function = bitset(1),
    Ast_Entity_Void = bitset(2),
    Ast_Entity_Atom = bitset(3),
    Ast_Entity_Struct = bitset(4),
    Ast_Entity_Enum = bitset(5),
    Ast_Entity_Record = bitset(6),
    Ast_Entity_Type = Ast_Entity_Void | Ast_Entity_Function | Ast_Entity_Atom | Ast_Entity_Struct | Ast_Entity_Enum,
};

struct Ast_Entity
{
    std::string name;
    virtual ~Ast_Entity() = default;
    virtual Ast_Entity_Kind kind() const = 0;
};

template <Ast_Entity_Kind K>
struct Ast_Entity_Impl : Ast_Entity
{
    constexpr static Ast_Entity_Kind Kind = K;

    Ast_Entity_Kind kind() const
    {
        return K;
    }
};

constexpr std::string_view ast_entity_kind_name(Ast_Entity_Kind kind)
{
    switch (kind)
    {
    case Ast_Entity_None:
        return "None";
    case Ast_Entity_Var:
        return "Var";
    case Ast_Entity_Function:
        return "Function";
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

// struct Ast_Entity
// {
//     std::string name;
//     Ast_Entity_Kind kind;
//     union {
//         Var var;
//         Signature_Type signature_type;
//         Void_Type void_type;
//         Atom_Type atom_type;
//         Struct_Type struct_type;
//         Enum_Type enum_type;
//     };

//     Ast_Entity() : kind{Ast_Entity_None} {}
//     Ast_Entity(std::string_view name, Var var) : name{name}, kind{Ast_Entity_Var}, var{var} {}
//     Ast_Entity(std::string_view name, Void_Type void_type) : name{name}, kind{Ast_Entity_Void}, void_type{void_type}
//     {} Ast_Entity(std::string_view name, Atom_Type atom_type) : name{name}, kind{Ast_Entity_Atom},
//     atom_type{atom_type} {} Ast_Entity(std::string_view name, Enum_Type enum_type) : name{name},
//     kind{Ast_Entity_Enum}, enum_type{enum_type} {}

//     Ast_Entity(std::string_view name, Signature_Type signature_type) :
//         name{name},
//         kind{Ast_Entity_Signature},
//         signature_type{signature_type}
//     {
//     }

//     Ast_Entity(std::string_view name, Struct_Type struct_type) :
//         name{name},
//         kind{Ast_Entity_Atom},
//         struct_type{struct_type}
//     {
//     }
// };

} // namespace bee

#endif
