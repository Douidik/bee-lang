#ifndef BEE_ENTITY_HPP
#define BEE_ENTITY_HPP

#include "stream.hpp"

namespace bee
{
struct Type;
    
struct Ast_Entity
{
    virtual ~Ast_Entity() {}
    virtual Ast_Dump_Stream &ast_dump(Ast_Dump_Stream &stream, s32 depth) const = 0;
};

} // namespace bee

#endif
