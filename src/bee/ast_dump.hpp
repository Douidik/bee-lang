#ifndef BEE_AST_DUMP_HPP
#define BEE_AST_DUMP_HPP

#include "error.hpp"
#include "stream.hpp"
#include <algorithm>
#include <fmt/format.h>

namespace bee
{
struct Ast;
struct Ast_Expr;
struct Ast_Entity;
struct Ast_Frame;

struct Ast_Dump_Header
{
    std::string_view name;
    s32 depth;
};

struct Ast_Dump : Stream
{
    Ast &ast;

    Ast_Dump(Ast &ast);
    void frame_dump(Ast_Dump_Header h, Ast_Frame *frame);
    void expr_dump(Ast_Dump_Header h, Ast_Expr *ast_expr);
    void entity_dump(Ast_Dump_Header h, Ast_Entity *ast_entity);
};

} // namespace bee

namespace fmt
{
using namespace bee;

template <>
struct formatter<Ast_Dump_Header>
{
    constexpr auto parse(format_parse_context &context)
    {
        return context.begin();
    }

    constexpr auto format(const Ast_Dump_Header &h, auto &context) const
    {
        return format_to(context.out(), "{:\t>{}} {}:", "", h.depth, h.name);
    }
};

} // namespace fmt

#endif
