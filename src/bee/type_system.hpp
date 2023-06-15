#ifndef BEE_TYPE_SYSTEM_HPP
#define BEE_TYPE_SYSTEM_HPP

#include "ast.hpp"
#include "bee_error.hpp"

namespace bee
{

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
    Ast_Entity *ssize_type;
    Ast_Entity *usize_type;
    Ast_Entity *void_type;

    u32 type_cast(Ast_Entity *from, Ast_Entity *into);
    u32 type_size(Ast_Entity *ast_entity);
    Ast_Entity *type_expr(Ast_Expr *ast_expr);
    Ast_Entity *type_entity(Ast_Entity *ast_entity);

    void type_std_def(Ast *ast);
    Ast_Entity *compose_atom(u32 desc, u32 size);

    Error errorf(std::string_view fmt, auto... args) const
    {
        return errorf("type_system error", fmt::format(fmt::runtime(fmt), args...));
    }
};

} // namespace bee

#endif
