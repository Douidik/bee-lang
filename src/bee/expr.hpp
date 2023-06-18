#ifndef BEE_EXPR_HPP
#define BEE_EXPR_HPP

#include "stream.hpp"
#include "token.hpp"
#include "type.hpp"
#include <string>
#include <variant>

namespace bee
{
struct Ast_Expr;
struct Ast_Entity;
struct Var;

enum Ast_Expr_Kind
{
    Ast_Expr_None,
    Ast_Expr_Unary,
    Ast_Expr_Binary,
    Ast_Expr_Nested,
    Ast_Expr_Scope,
    Ast_Expr_If,
    Ast_Expr_Return,
    Ast_Expr_Id,
    Ast_Expr_Def,
    Ast_Expr_Char,
    Ast_Expr_Str,
    Ast_Expr_Int,
    Ast_Expr_Float,
    Ast_Expr_Function,
    Ast_Expr_Argument,
    Ast_Expr_Invoke,
};

using Compound_Expr = std::vector<Ast_Expr *>;

enum Order_Expr : u32
{
    Prev_Expr,
    Post_Expr,
};

struct Ast_Expr
{
    virtual ~Ast_Expr() = default;
    virtual Ast_Expr_Kind kind() const = 0;
};

template <Ast_Expr_Kind K>
struct Ast_Expr_Impl : Ast_Expr
{
    constexpr static Ast_Expr_Kind Kind = K;

    Ast_Expr_Kind kind() const
    {
        return K;
    }
};

struct None_Expr : Ast_Expr_Impl<Ast_Expr_None>
{
};

struct Unary_Expr : Ast_Expr_Impl<Ast_Expr_Unary>
{
    Token op;
    Order_Expr order;
    Ast_Expr *expr;
};

struct Binary_Expr : Ast_Expr_Impl<Ast_Expr_Binary>
{
    Token op;
    Ast_Entity *type;
    Ast_Expr *prev;
    Ast_Expr *post;
};

struct Nested_Expr : Ast_Expr_Impl<Ast_Expr_Nested>
{
    Ast_Expr *expr;
};

struct Scope_Expr : Ast_Expr_Impl<Ast_Expr_Scope>
{
    Compound_Expr *compound;
};

struct If_Expr : Ast_Expr_Impl<Ast_Expr_If>
{
    Ast_Expr *cond;
};

struct Return_Expr : Ast_Expr_Impl<Ast_Expr_Return>
{
    Ast_Expr *expr;
};

struct Id_Expr : Ast_Expr_Impl<Ast_Expr_Id>
{
    Token name;
    Ast_Entity *entity;
};

struct Def_Expr : Ast_Expr_Impl<Ast_Expr_Def>
{
    Var *var;
    Ast_Expr *expr;
    Token op;
    Token name;
    Def_Expr *next;
};

struct Char_Expr : Ast_Expr_Impl<Ast_Expr_Char>
{
    char data;
};

struct Str_Expr : Ast_Expr_Impl<Ast_Expr_Str>
{
    // TODO! define const char literal as entity
    std::string_view data;
};

struct Int_Expr : Ast_Expr_Impl<Ast_Expr_Int>
{
    u64 data;
    u32 size;
};

struct Float_Expr : Ast_Expr_Impl<Ast_Expr_Float>
{
    f64 data;
    u32 size;
};

struct Function_Expr : Ast_Expr_Impl<Ast_Expr_Function>
{
    Signature *signature;
    Scope_Expr *scope;
};

struct Argument_Expr : Ast_Expr_Impl<Ast_Expr_Argument>
{
    Ast_Expr *expr;
    Argument_Expr *next;
};

struct Invoke_Expr : Ast_Expr_Impl<Ast_Expr_Invoke>
{
    Ast_Expr *function;
    Argument_Expr *args;
};

constexpr std::string_view ast_expr_kind_name(Ast_Expr_Kind kind)
{
    switch (kind)
    {
    case Ast_Expr_None:
        return "Ast_Expr_None";
    case Ast_Expr_Unary:
        return "Ast_Expr_Unary";
    case Ast_Expr_Binary:
        return "Ast_Expr_Binary";
    case Ast_Expr_Nested:
        return "Ast_Expr_Nested";
    case Ast_Expr_Scope:
        return "Ast_Expr_Scope";
    case Ast_Expr_If:
        return "Ast_Expr_If";
    case Ast_Expr_Return:
        return "Ast_Expr_Return";
    case Ast_Expr_Id:
        return "Ast_Expr_Id";
    case Ast_Expr_Def:
        return "Ast_Expr_Def";
    case Ast_Expr_Char:
        return "Ast_Expr_Char";
    case Ast_Expr_Str:
        return "Ast_Expr_Str";
    case Ast_Expr_Int:
        return "Ast_Expr_Int";
    case Ast_Expr_Float:
        return "Ast_Expr_Float";
    case Ast_Expr_Function:
        return "Ast_Expr_Function";
    case Ast_Expr_Invoke:
        return "Ast_Expr_Invoke";
    case Ast_Expr_Argument:
        return "Ast_Expr_Argument";
    default:
        return "?";
    }
}

constexpr std::string_view order_expr_name(Order_Expr order)
{
    switch (order)
    {
    case Prev_Expr:
        return "prev";
    case Post_Expr:
        return "post";
    default:
        return "?";
    }
}

} // namespace bee

#endif
