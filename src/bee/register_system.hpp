#ifndef BEE_REGISTER_SYSTEM_HPP
#define BEE_REGISTER_SYSTEM_HPP

#include "core.hpp"
#include "register.hpp"
#include <set>
#include <span>
#include <unordered_map>
#include <vector>

namespace bee
{
struct Ast;
struct Ast_Expr;
struct Scope_Expr;
struct Unary_Expr;
struct Binary_Expr;
struct Nested_Expr;
struct Return_Expr;
struct Id_Expr;
struct Var_Expr;
struct Var;
struct Function_Expr;
struct Type_System;

struct Register_Allocator
{
    struct Sort_By_Start
    {
        bool operator()(const Var *v, const Var *w) const;
    };
    struct Sort_By_End
    {
        bool operator()(const Var *v, const Var *w) const;
    };

    Function_Expr *function;
    std::span<Register> regs;
    Type_System &type_system;
    std::set<Register *> regs_free;
    std::set<Var *, Sort_By_Start> vars;
    std::set<Var *, Sort_By_End> active;
    u32 time;
    u32 sp;

    Register_Allocator(Function_Expr *function, std::span<Register> regs, Type_System &type_system);
    void allocate();
    void expire_vars(Var *v);
    void spill_var(Var *v);
    void parse_intervals(Ast_Expr *expr);
};

} // namespace bee

#endif
