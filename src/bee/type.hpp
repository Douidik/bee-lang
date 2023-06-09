#ifndef BEE_TYPE_HPP
#define BEE_TYPE_HPP

#include "core.hpp"
#include "entity.hpp"

namespace bee
{

enum Type_Kind
{
    Type_Void,
    Type_Atom,
    Type_Struct,
    Type_Enum,
};

struct Void
{
};

enum Atom_Desc : u32
{
    Atom_Raw = 0,
    Atom_Error = 1 << 0,
    Atom_Signed = 1 << 1,
    Atom_Float = 1 << 2,
};

struct Atom
{
    Atom_Desc desc;
    u32 size;
};

struct Struct
{
};

struct Type : Ast_Entity
{
    Type_Kind kind;
    union {
        Void void_;
        Atom atom_;
        Struct struct_;
    };

    static Type make_void();
    static Type make_atom(Atom_Desc desc, u32 size);
    Ast_Dump_Stream &ast_dump(Ast_Dump_Stream &stream, s32 depth) const;
};

constexpr std::string_view atom_desc_name(Atom_Desc desc)
{
    switch (desc)
    {
    case Atom_Raw:
        return "Atom_Raw";
    case Atom_Error:
        return "Atom_Error";
    case Atom_Signed:
        return "Atom_Signed";
    case Atom_Float:
        return "Atom_Float";
    default:
        return "?";
    }
}

// struct Type : Ast_Entity
// {
//     virtual Type_Kind kind() const = 0;
//     virtual u32 type_size() const = 0;
//     virtual bool does_cast(const Type *type) const = 0;

//     Type *type()
//     {
//         return this;
//     }
// };

// struct Void : Type
// {
//     Ast_Dump_Stream &ast_dump(Ast_Dump_Stream &stream, s32 depth) const
//     {
//         return stream.print(depth, "Void");
//     }

//     Type_Kind kind() const
//     {
//         return Type_Void;
//     }

//     u32 type_size() const
//     {
//         return 0;
//     }

//     bool does_cast(const Type *type) const
//     {
//         return type->kind() == Type_Void;
//     }
// };

// inline static Void Void_Instance = Void{};

// enum Atom_Desc : u32
// {
//     Atom_Raw = 0,
//     Atom_Error = 1 << 0,
//     Atom_Signed = 1 << 1,
//     Atom_Float = 1 << 2,
// };
// constexpr std::string_view atom_desc_name(Atom_Desc desc);

// struct Atom : Type
// {
//     u32 size;
//     Atom_Desc desc;

//     Atom(Atom_Desc desc, u32 size) :  desc{desc}, size{size} {}

//     Ast_Dump_Stream &ast_dump(Ast_Dump_Stream &stream, s32 depth) const
//     {
//         return stream.print(depth, "Atom (size: {:d}, desc: {:s})", size, atom_desc_name(desc));
//     }

//     Type_Kind kind() const
//     {
//         return Type_Atom;
//     }

//     u32 type_size() const
//     {
//         return size;
//     }

//     bool does_cast(const Type *type) const
//     {
//         return type->kind() == Type_Atom;
//     }
// };

} // namespace bee

#endif