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

    Ast_Entity *find(std::string_view name);
};

struct Ast
{
    Dyn_Arena<Ast_Expr, 64> exprs;
    Arena<Ast_Frame, 32> stack;
    Ast_Frame *frame = NULL;

    Ast_Expr *find(std::string_view name);
    Ast_Frame *stack_push();
    Ast_Frame *stack_pop();

    template <typename T>
    Ast_Entity *entity_push(std::string_view name, auto ...args)
    {
	return &(frame->defs[name] = Ast_Entity(T{args...}));
    }

    template <typename T>
    Ast_Expr *expr_push(auto... args)
    {
        return &exprs.push(T{args...});
    }
};

} // namespace bee

#endif
