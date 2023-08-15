#ifndef BEE_STRUCT_HPP
#define BEE_STRUCT_HPP

#include "entity.hpp"

namespace bee
{
struct Frame;

struct Struct_Type : Ast_Entity_Impl<Ast_Entity_Enum>
{
    Frame *frame;
};

} // namespace bee

#endif
