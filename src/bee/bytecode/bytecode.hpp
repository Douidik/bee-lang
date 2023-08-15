#ifndef BEE_BYTECODE_HPP
#define BEE_BYTECODE_HPP

#include "arena.hpp"
#include "error.hpp"
#include "fwd.hpp"
#include "instruction.hpp"
#include <fmt/format.h>
#include <span>
#include <vector>

namespace bee
{

struct Ast;

struct Bytecode
{
    Ast *ast;
    std::vector<Bc_Instruction> instructions;
    Dyn_Arena<Bc_Object, 512> objects;
    Bc_Object none;

    Bytecode(Ast *ast);
    std::span<Bc_Instruction> gen();
    Bc_Object *gen_expr(Ast_Expr *expr);
    Bc_Object *gen_unary(Unary_Expr *unary);
    Bc_Object *gen_binary(Binary_Expr *binary);
    Bc_Object *gen_scope(Scope_Expr *scope);
    Bc_Object *gen_var(Var_Expr *def);
    Bc_Object *gen_var_id(Id_Expr *id);
    Bc_Object *gen_function(Function_Expr *def);
    Bc_Object *gen_invoke(Invoke_Expr *invoke);
    Bc_Object *gen_atom(Ast_Entity *entity, void *data);
    Bc_Object *gen_return(Return_Expr *return_expr);
    Bc_Object *gen_if(If_Expr *if_expr);

    // void push_binary(Bc_Instruction instruction);
    void push_instruction(Bc_Instruction instruction);
    Bc_Object *object_var(Var *var);
    Bc_Object *object_imm(Ast_Entity *type);

    Error errorf(std::string_view fmt, auto... args)
    {
        return Error{"vm error", fmt::format(fmt::runtime(fmt), args...)};
    }
};

} // namespace bee

#endif
