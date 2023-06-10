#include "type.hpp"
#include "ast.hpp"
#include "entity.hpp"
#include "expr.hpp"

namespace bee
{

// Ast_Dump_Stream &Type::ast_dump(Ast_Dump_Stream &stream, s32 depth) const
// {
//     switch (kind)
//     {
//     case Type_Void:
//         return stream.print(depth, "Type_Void");

//     case Type_Atom:
//         return stream.print(depth, "Type_Atom (desc: {:s}, size {:d})", atom_desc_name(atom_.desc), atom_.size);

//     case Type_Struct:
//     case Type_Enum:
//         return stream.print(depth, "<unimplemented>");
//     }
// }

} // namespace bee
