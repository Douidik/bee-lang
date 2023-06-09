#include "type.hpp"

namespace bee
{

Type Type::make_void()
{
    Type type = Type{};
    type.kind = Type_Void;
    type.void_ = Void{};

    return type;
}

Type Type::make_atom(Atom_Desc desc, u32 size)
{
    Type type = Type{};
    type.kind = Type_Atom;
    type.atom_ = Atom{desc, size};

    return type;
}

Ast_Dump_Stream &Type::ast_dump(Ast_Dump_Stream &stream, s32 depth) const
{
    switch (kind)
    {
    case Type_Void:
        return stream.print(depth, "Type_Void");

    case Type_Atom:
        return stream.print(depth, "Type_Atom (desc: {:s}, size {:d})", atom_desc_name(atom_.desc), atom_.size);

    case Type_Struct:
    case Type_Enum:
        return stream.print(depth, "<unimplemented>");
    }
}

} // namespace bee
