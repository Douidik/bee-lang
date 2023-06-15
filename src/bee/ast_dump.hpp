#ifndef BEE_AST_DUMP_HPP
#define BEE_AST_DUMP_HPP

#include "stream.hpp"

namespace bee
{
struct Ast_Expr;
struct Ast_Entity;
struct Ast_Frame;
struct Ast;

struct Ast_Dump : Standard_Stream
{
    Ast_Dump(Ast &ast);

    void stack_dump(Ast_Frame *frame, s32 depth);
    void expr_dump(Ast_Expr *ast_expr, s32 depth);
    void entity_dump(Ast_Entity *ast_entity, s32 depth);

    Ast_Dump &print(s32 depth, std::string_view fmt, auto... args)
    {
        std_print("{:\t>{}}- ", "", depth);
        std_print(fmt, args...);
        return *this;
    }
};

} // namespace bee
#endif
