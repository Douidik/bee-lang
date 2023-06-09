#ifndef BEE_AST_DUMP_HPP
#define BEE_AST_DUMP_HPP

#include "stream.hpp"

namespace bee
{
struct Ast_Expr;
struct Ast_Entity;

struct Ast_Dump : Standard_Stream
{
    void dump_expr(Ast_Expr *ast_expr, s32 depth);
    void dump_entity(Ast_Entity *ast_expr, s32 depth);

    Ast_Dump &print(s32 depth, std::string_view fmt, auto... args)
    {
        standard_print("{:\t>{}}- ", "", depth);
        standard_print(fmt, args...);
        return *this;
    }
};
    
} // namespace bee
#endif
