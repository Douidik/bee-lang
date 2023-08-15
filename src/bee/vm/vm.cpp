#include "vm.hpp"
#include "ast.hpp"
#include "type.hpp"
#include "var.hpp"
#include <cmath>
#include <type_traits>

namespace bee
{

Vm::Vm(Ast *ast) :
    ast{ast},
    sp{0},
    type_system{ast->type_system},
    vm_none{
        type_system.void_type,
        NULL,
    }
{
}

s32 Vm::run()
{
    for (Ast_Expr *expr : *ast->main_scope->compound)
    {
        run_expr(expr);
    }

    Ast_Entity *main = ast->main_frame->find_def("main");
    if (!main or main->kind() != Ast_Entity_Function)
        throw errorf("no entry point defined in program, consider the implementation of 'main :: () -> s32'");
    Scope_Expr *main_scope = ast->main_frame->find_function((Function *)main);
    return *(s32 *)run_scope(main_scope).ref;

    // Ast_Entity *main = ast->main_frame->find_def("main");
    // if (!main or main->kind() != Ast_Entity_Function)
    // 	throw errorf("no entry point defined in program");
    // Scope_Expr *main_scope = Ast_Func

    // run_scope(
}

Vm_Object Vm::run_expr(Ast_Expr *expr)
{
    switch (expr->kind())
    {
    case Ast_Expr_Unary:
        return run_unary((Unary_Expr *)expr);

    case Ast_Expr_Binary:
        return run_binary((Binary_Expr *)expr);

    case Ast_Expr_Nested:
        return run_expr(((Nested_Expr *)expr)->expr);

    case Ast_Expr_Scope:
        return run_scope((Scope_Expr *)expr);

    case Ast_Expr_Id:
        return run_var_id((Id_Expr *)expr);

    case Ast_Expr_Var:
        return run_var((Var_Expr *)expr);

    case Ast_Expr_Char: {
        Char_Expr *char_expr = (Char_Expr *)expr;
        return run_atom(type_system.expr_type(char_expr), &char_expr->data);
    }

    case Ast_Expr_Int: {
        Int_Expr *int_expr = (Int_Expr *)expr;
        return run_atom(type_system.expr_type(int_expr), &int_expr->data);
    }

    case Ast_Expr_Float: {
        Float_Expr *float_expr = (Float_Expr *)expr;
        return run_atom(type_system.expr_type(float_expr), &float_expr->data);
    }

    case Ast_Expr_Return:
        return run_return((Return_Expr *)expr);

    case Ast_Expr_Function:
        return run_function((Function_Expr *)expr);

    case Ast_Expr_Invoke:
        return run_invoke((Invoke_Expr *)expr);

    case Ast_Expr_If:
        return run_if((If_Expr *)expr);

    case Ast_Expr_For:
    case Ast_Expr_For_Range:
    case Ast_Expr_For_While:

    default:
        throw errorf("TODO! run_expr() not implemented for '{:s}'", ast_expr_kind_name(expr->kind()));
    }

    return vm_none;
}

Vm_Atom Vm::vm_atom(Atom_Type *type, u8 *ref)
{
    switch (type->desc)
    {
    case Atom_Raw:
        switch (type->size)
        {
        case 1:
            return (u8 *)ref;
        case 2:
            return (u16 *)ref;
        case 4:
            return (u32 *)ref;
        case 8:
            return (u64 *)ref;
        }
        return {};

    case Atom_Signed:
        switch (type->size)
        {
        case 1:
            return (s8 *)ref;
        case 2:
            return (s16 *)ref;
        case 4:
            return (s32 *)ref;
        case 8:
            return (s64 *)ref;
        }
        return {};

    case Atom_Float:
        switch (type->size)
        {
        case 4:
            return (f32 *)ref;
        case 8:
            return (f64 *)ref;
        }
        return {};
    }
    return {};
}

Vm_Object Vm::run_unary(Unary_Expr *unary)
{
    Vm_Object object = run_expr(unary->expr);
    if (object.type->kind() != Ast_Entity_Atom)
    {
        throw errorf("cannot perform unary operation on non-atom expression of type '{:s}'", object.type->name);
    }
    Vm_Atom atom = vm_atom((Atom_Type *)object.type, object.ref);

#define vm_prev_op(op)                                                  \
    [&](auto &&v) -> Vm_Object {                                        \
        auto w = op(*v);                                                \
        return Vm_Object{object.type, stack_push((u8 *)&w, sizeof(w))}; \
    }

#define vm_post_op(op)                                                  \
    [&](auto &&v) -> Vm_Object {                                        \
        auto w = (*v)op;                                                \
        return Vm_Object{object.type, stack_push((u8 *)&w, sizeof(w))}; \
    }

    switch (unary->op.type)
    {
    case Token_Add:
        return std::visit(vm_prev_op(+), atom);

    case Token_Sub:
        return std::visit(vm_prev_op(-), atom);

    case Token_Decrement: {
        switch (unary->order)
        {
        case Prev_Expr:
            return std::visit(vm_prev_op(--), atom);
        case Post_Expr:
            return std::visit(vm_post_op(--), atom);
        }
    }

    case Token_Increment: {
        switch (unary->order)
        {
        case Prev_Expr:
            return std::visit(vm_prev_op(++), atom);
        case Post_Expr:
            return std::visit(vm_post_op(++), atom);
        }
    }

    default:
        throw errorf("TODO! run_unary() not implemented for '{:s}'", token_typename(unary->op.type));
    }

#undef vm_prev_op
#undef vm_post_op
}

template <typename T>
concept Any = true;

template <typename T>
concept Is_Int = std::integral<std::decay_t<T>>;

template <class... Ts>
struct Visit_Pack : Ts...
{
    using Ts::operator()...;
};
template <class... Ts>
Visit_Pack(Ts...) -> Visit_Pack<Ts...>;

Vm_Object Vm::run_binary(Binary_Expr *binary)
{
    Vm_Object object_prev = run_expr(binary->prev);
    Vm_Object object_post = run_expr(binary->post);
    if ((object_prev.type->kind() & object_post.type->kind()) != Ast_Entity_Atom)
    {
        throw errorf("cannot perform binary operation on non-atom expression of type '{:s}', '{:s}'",
                     object_prev.type->name, object_post.type->name);
    }
    Vm_Atom atom_prev = vm_atom((Atom_Type *)object_prev.type, object_prev.ref);
    Vm_Atom atom_post = vm_atom((Atom_Type *)object_post.type, object_post.ref);

#define vm_op(op, V, W)                                                  \
    [&](V auto &&v, W auto &&w) -> Vm_Object {                           \
        auto x = (*v)op(*w);                                             \
        return Vm_Object{binary->type, stack_push((u8 *)&x, sizeof(x))}; \
    }

    auto vm_op_error = [&](auto, auto) -> Vm_Object {
        throw errorf("cannot perform binary operation '{:s}' with expressions of type '{:s}', '{:s}'",
                     token_typename(binary->op.type), object_prev.type->name, object_post.type->name);
    };

    switch (binary->op.type)
    {
    case Token_Assign:
        return std::visit(vm_op(=, Any, Any), atom_prev, atom_post);

    case Token_And:
        return std::visit(vm_op(&&, Any, Any), atom_prev, atom_post);
    case Token_Or:
        return std::visit(vm_op(||, Any, Any), atom_prev, atom_post);
    case Token_Add:
        return std::visit(vm_op(+, Any, Any), atom_prev, atom_post);
    case Token_Sub:
        return std::visit(vm_op(-, Any, Any), atom_prev, atom_post);
    case Token_Mul:
        return std::visit(vm_op(*, Any, Any), atom_prev, atom_post);
    case Token_Div:
        return std::visit(vm_op(/, Any, Any), atom_prev, atom_post);
    case Token_Mod:
        return std::visit(Visit_Pack{vm_op(&, Is_Int, Is_Int), vm_op_error}, atom_prev, atom_post);

    case Token_Bin_And:
        return std::visit(Visit_Pack{vm_op(&, Is_Int, Is_Int), vm_op_error}, atom_prev, atom_post);
    case Token_Bin_Or:
        return std::visit(Visit_Pack{vm_op(|, Is_Int, Is_Int), vm_op_error}, atom_prev, atom_post);
    case Token_Bin_Xor:
        return std::visit(Visit_Pack{vm_op(^, Is_Int, Is_Int), vm_op_error}, atom_prev, atom_post);
    case Token_Shift_L:
        return std::visit(Visit_Pack{vm_op(<<, Is_Int, Is_Int), vm_op_error}, atom_prev, atom_post);
    case Token_Shift_R:
        return std::visit(Visit_Pack{vm_op(>>, Is_Int, Is_Int), vm_op_error}, atom_prev, atom_post);

    case Token_Eq:
        return std::visit(vm_op(==, Any, Any), atom_prev, atom_post);
    case Token_Not_Eq:
        return std::visit(vm_op(!=, Any, Any), atom_prev, atom_post);
    case Token_Less:
        return std::visit(vm_op(<, Any, Any), atom_prev, atom_post);
    case Token_Less_Eq:
        return std::visit(vm_op(<=, Any, Any), atom_prev, atom_post);
    case Token_Greater:
        return std::visit(vm_op(>, Any, Any), atom_prev, atom_post);
    case Token_Greater_Eq:
        return std::visit(vm_op(>=, Any, Any), atom_prev, atom_post);

    default:
        throw errorf("TODO! run_binary() not implemented for '{:s}'", token_typename(binary->op.type));
    }
#undef vm_op
}

Vm_Object Vm::run_scope(Scope_Expr *scope)
{
    Vm_Object object = vm_none;
    u64 bsp = sp;
    ast->push_frame(scope->frame);

    for (Ast_Expr *expr : *scope->compound)
    {
        object = run_expr(expr);
        if (object.interrupt != Vm_Interrupt_None)
            break;
    }

    ast->pop_frame();
    sp = bsp;
    return object;
}

Vm_Object Vm::run_var(Var_Expr *def)
{
    Vm_Object object = run_expr(def->expr);
    if (type_system.cast_type(object.type, def->var->type) >= Type_Cast_Transmuted)
    {
        throw errorf("variable definition expression reduces to '{:s}' instead of '{:s}'", object.type->name,
                     def->var->type->name);
    }
    ast->frame->push_ref(def->var, object.ref);

    if (def->next != NULL)
        return run_var(def->next);
    return object;
}

Vm_Object Vm::run_var_id(Id_Expr *id)
{
    if (id->entity->kind() != Ast_Entity_Var)
    {
        throw errorf("'{:s}' does not reference a variable", id->name.expr);
    }

    Var *var = (Var *)id->entity;
    u8 *ref = ast->frame->find_ref(var);
    if (!ref)
    {
        throw errorf("cannot get the reference for '{:s}'", var->name);
    }

    return Vm_Object{var->type, ref};
}

Vm_Object Vm::run_function(Function_Expr *def)
{
    ast->frame->push_function(def->function, def->scope);
    return vm_none;
}

Vm_Object Vm::run_invoke(Invoke_Expr *invoke)
{
    u64 bsp = sp;
    Function *function = invoke->function;

    Scope_Expr *scope = ast->frame->find_function(function);
    if (!scope)
    {
        throw errorf("no definition found for function '{:s}'", function->name);
    }

    ast->push_frame(new Frame{});
    init_params(function->params, invoke->args);
    // run_var(function->params);
    Vm_Object return_object = run_scope(scope);

    for (auto [entity, ref] : ast->frame->refs)
    {
        Ast_Entity *type = type_system.entity_type(entity);
        if (type->kind() & Ast_Entity_Atom)
        {
            std::visit(
                [&](auto &&v) {
                    fmt::print("{}: {} = {}\n", entity->name, type->name, *v);
                },
                vm_atom((Atom_Type *)type, ref));
        }
    }

    for (auto [function, scope] : ast->frame->binds)
    {
        fmt::print("'{:s}'\n{:p}", function->name, fmt::ptr(scope));
    }

    ast->pop_frame();
    sp = bsp;
    return function->type->kind() != Ast_Entity_Void ? return_object : vm_none;
}

Vm_Object Vm::run_return(Return_Expr *return_expr)
{
    Vm_Object object = run_expr(return_expr->expr);
    return Vm_Object{object.type, object.ref, Vm_Return};
}

Vm_Object Vm::run_if(If_Expr *if_expr)
{
    Vm_Object return_object = vm_none;
    u32 bsp = sp;
    ast->push_frame(if_expr->frame);

    Vm_Object object = run_expr(if_expr->condition);
    Vm_Atom atom = vm_atom((Atom_Type *)object.type, object.ref);
    bool condition;

    std::visit(
        [&condition](auto &&v) {
            condition = static_cast<bool>(*v);
        },
        atom);

    if (condition)
        return_object = run_scope(if_expr->scope_if);
    else if (if_expr->scope_else != NULL)
        return_object = run_scope(if_expr->scope_else);

    ast->pop_frame();
    sp = bsp;
    return return_object;
}

Vm_Object Vm::run_atom(Ast_Entity *entity, void *data)
{
    u32 size = type_system.size_type(entity);
    u8 *ref = stack_push((u8 *)data, size);
    return Vm_Object{entity, ref};
}

// TODO! Implement default argument parameters
void Vm::init_params(Var_Expr *param, Argument_Expr *argument)
{
    if (!param or !argument)
        return;

    Vm_Object object = run_expr(argument->expr);
    if (type_system.cast_type(object.type, param->var->type) >= Type_Cast_Transmuted)
    {
        throw errorf("cannot cast argument of type '{}' to parameter '{}: {}'", object.type->name, param->var->name,
                     param->var->type->name);
    }

    ast->frame->push_ref(param->var, object.ref);
    init_params(param->next, argument->next);
}

u8 *Vm::stack_push(u8 *data, usize size)
{
    if (sp + size > std::size(stack))
        throw errorf("stack overflow (sp > {})", std::size(stack));
    data = (u8 *)std::memcpy(&stack[sp], data, size);
    return sp += size, data;
}

u8 *Vm::stack_pop(usize size)
{
    if (sp - size < 0)
        throw errorf("stack underflow (sp < 0)");
    return &stack[sp -= size];
}

} // namespace bee
