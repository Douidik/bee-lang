#ifndef BEE_AST_HPP
#define BEE_AST_HPP

#include "arena.hpp"
#include "entity.hpp"
#include "expr.hpp"
#include <unordered_map>

namespace bee
{

struct Frame
{
    std::unordered_map<std::string_view, Ast_Entity *> defs;
    Frame *owner;

    void free();
    Ast_Entity *find(std::string_view name);
};
    
struct Ast
{
    std::vector<Ast_Expr *> exprs;
    Arena<Frame, 32> stack;
    Frame *frame = NULL;

    void free();
    Ast_Expr *find(std::string_view name);
    Ast_Expr *expr_push(Ast_Expr *expr);
    Ast_Entity *entity_push(Ast_Entity *entity);
    Frame *stack_push();
    Frame *stack_pop();
};

} // namespace bee

#endif
