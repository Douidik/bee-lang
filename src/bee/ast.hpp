#ifndef BEE_AST_HPP
#define BEE_AST_HPP

#include "entity.hpp"
#include "expr.hpp"
#include "frame.hpp"
#include "type_system.hpp"
#include <unordered_map>

namespace bee
{

const u32 Ast_Frame_Limit = 256;

struct Ast
{
    Mem_Arena<1024> exprs;
    std::set<Frame *> frames;
    std::deque<Compound_Expr> compounds;
    Frame *frame = NULL;
    Frame *main_frame;
    Scope_Expr *main_scope;
    Type_System type_system;
    u32 expr_count;

    ~Ast();
    Ast_Expr *find(std::string_view name);
    Frame *push_frame(Frame *f);
    Frame *pop_frame();
    Compound_Expr *push_compound(Compound_Expr compound);

    template <typename T>
    T *push_expr(T expr)
    {
        static_assert(std::is_base_of_v<Ast_Expr, std::decay_t<T>>, "type is not an expression");
        return expr_count++, exprs.push<std::decay_t<T>>(std::move(expr));
    }

    Error errorf(std::string_view fmt, auto... args)
    {
        return Error{"ast error", fmt::format(fmt::runtime(fmt), args...)};
    }
};

} // namespace bee

#endif
