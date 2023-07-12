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

    template <typename T>
    T *push(T *entity)
    {
        static_assert(std::is_base_of_v<Ast_Entity, std::decay_t<T>>, "type is not an entity");
        return (T *)(defs[entity->name] = entity);
    }
};

struct Ast
{
    Mem_Arena<1024> exprs;
    std::set<Ast_Frame *> frames;
    std::deque<Compound_Expr> compounds;
    Ast_Frame *frame = NULL;
    Ast_Frame *main_frame;
    Scope_Expr *main_scope;

    ~Ast();
    Ast_Expr *find(std::string_view name);
    Ast_Frame *push_frame(Ast_Frame *f);
    Ast_Frame *pop_frame();
    Compound_Expr *push_compound(Compound_Expr compound);

    template <typename T>
    T *push_expr(T expr)
    {
        static_assert(std::is_base_of_v<Ast_Expr, std::decay_t<T>>, "type is not an expression");
        return exprs.push<std::decay_t<T>>(std::move(expr));
    }
};

} // namespace bee

#endif
