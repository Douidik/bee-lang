#ifndef BEE_AST_HPP
#define BEE_AST_HPP

#include "entity.hpp"
#include "expr.hpp"
#include <unordered_map>

namespace bee
{

struct Ast_Frame
{
    std::unordered_map<std::string_view, Ast_Entity> defs;
    Ast_Frame *owner;

    template <typename T>
    Ast_Entity *push(std::string_view name, auto... args)
    {
        return &(defs[name] = Ast_Entity(std::string{name}, T{args...}));
    }
 
    Ast_Entity *find(std::string_view name);
};

struct Ast
{
    Dyn_Arena<Ast_Expr, 64> exprs;
    Dyn_Arena<Compound_Expr, 32> compounds;
    Arena<Ast_Frame, 32> stack;
    Ast_Frame *frame = NULL;

    Ast_Expr *find(std::string_view name);
    Ast_Frame *stack_push();
    Ast_Frame *stack_pop();
    Compound_Expr *compound_push(const Compound_Expr &&compound);

    template <typename T>
    Ast_Expr *expr_push(auto... args)
    {
        return &exprs.push(T{args...});
    }
};

} // namespace bee

#endif
