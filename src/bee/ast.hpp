#ifndef BEE_AST_HPP
#define BEE_AST_HPP

#include "entity.hpp"
#include "expr.hpp"
#include <unordered_map>

namespace bee
{

struct Ast_Frame
{
    std::unordered_map<std::string_view, Ast_Entity *> defs;
    Ast_Frame *owner;

    ~Ast_Frame();
    Ast_Entity *find(std::string_view name);
    Ast_Entity *push(Ast_Entity *entity);
};

struct Ast
{
    Mem_Arena<1024> exprs;
    Arena<Ast_Frame, 32> stack;
    Dyn_Arena<Compound_Expr, 32> compounds;
    Ast_Frame *frame = NULL;
    Ast_Frame *main_frame;
    Scope_Expr *main_scope;

    Ast_Expr *find(std::string_view name);
    Ast_Frame *stack_push();
    Ast_Frame *stack_pop();
    Compound_Expr *compound_push(const Compound_Expr &&compound);

    template <typename T>
    T *expr_push(T expr)
    {
        static_assert(std::is_base_of_v<Ast_Expr, std::decay_t<T>>, "type is not an expression");
        return exprs.push<std::decay_t<T>>(std::move(expr));
    }
};

} // namespace bee

#endif
