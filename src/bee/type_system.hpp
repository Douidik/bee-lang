#ifndef BEE_TYPE_SYSTEM_HPP
#define BEE_TYPE_SYSTEM_HPP

#include "bee_error.hpp"

namespace bee
{
struct Ast;
struct Ast_Expr;
struct Ast_Entity;

enum Type_Cast : u32
{
    Type_Cast_Same = 1 << 0,
    Type_Cast_Inferred = 1 << 1,
    Type_Cast_Narrowed = 1 << 2,
    Type_Cast_Transmuted = 1 << 3,
    Type_Cast_Error = 1 << 4,
};

struct Type_System
{
    Ast_Entity *f16_type;
    Ast_Entity *f32_type;
    Ast_Entity *f64_type;
    Ast_Entity *s8_type;
    Ast_Entity *s16_type;
    Ast_Entity *s32_type;
    Ast_Entity *s64_type;
    Ast_Entity *u8_type;
    Ast_Entity *u16_type;
    Ast_Entity *u32_type;
    Ast_Entity *u64_type;
    Ast_Entity *char_type;
    Ast_Entity *bool_type;
    Ast_Entity *ssize_type;
    Ast_Entity *usize_type;
    Ast_Entity *void_type;

    u32 cast_type(Ast_Entity *from, Ast_Entity *into);
    u32 size_type(Ast_Entity *ast_entity);
    Ast_Entity *expr_type(Ast_Expr *ast_expr);
    Ast_Entity *entity_type(Ast_Entity *ast_entity);

    void std_types(Ast *ast);
    Ast_Entity *compose_atom(u32 desc, u32 size);

    Error errorf(std::string_view fmt, auto... args) const
    {
        return Error{"type_system error", fmt::format(fmt::runtime(fmt), args...)};
    }
};

} // namespace bee

#endif
