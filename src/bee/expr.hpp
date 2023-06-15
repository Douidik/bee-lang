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
struct Proc;

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
    Ast_Expr_Proc,
    Ast_Expr_Argument,
    Ast_Expr_Invoke,
};

using Compound_Expr = std::vector<Ast_Expr *>;

enum Order_Expr : u32
{
    Prev_Expr,
    Post_Expr,
};

struct None_Expr
{
};

struct Unary_Expr
{
    Token op;
    Order_Expr order;
    Ast_Expr *expr;
};

struct Binary_Expr
{
    Token op;
    Ast_Entity *type;
    Ast_Expr *prev;
    Ast_Expr *post;
};

struct Nested_Expr
{
    Ast_Expr *expr;
};

struct Scope_Expr
{
    Compound_Expr *compound;
};

struct If_Expr
{
    Ast_Expr *cond;
};

struct Return_Expr
{
    Ast_Expr *expr;
};

struct Id_Expr
{
    Token name;
    Ast_Entity *entity;
};

struct Def_Expr
{
    Ast_Entity *entity;
    Ast_Expr *expr;
    Token op;
    Token name;
    Ast_Expr *next;
};

struct Char_Expr
{
    char data;
};

struct Str_Expr
{
    // TODO! define const char literal as entity
    std::string_view data;
};

struct Int_Expr
{
    u64 data;
    u32 size;
};

struct Float_Expr
{
    f64 data;
    u32 size;
};

struct Proc_Expr
{
    Ast_Entity *signature;
    Ast_Expr *scope;
};

struct Argument_Expr
{
    Ast_Expr *expr;
    Ast_Expr *next;
};

struct Invoke_Expr
{
    Ast_Expr *proc;
    Ast_Expr *args;
};

struct Ast_Expr
{
    Ast_Expr_Kind kind;

    union {
        None_Expr none_expr;
        Unary_Expr unary_expr;
        Binary_Expr binary_expr;
        Nested_Expr nested_expr;
        Scope_Expr scope_expr;
        If_Expr if_expr;
        Return_Expr return_expr;
        Id_Expr id_expr;
        Def_Expr def_expr;
        Char_Expr char_expr;
        Str_Expr str_expr;
        Int_Expr int_expr;
        Float_Expr float_expr;
        Proc_Expr proc_expr;
        Argument_Expr argument_expr;
        Invoke_Expr invoke_expr;
    };

    Ast_Expr() : kind{Ast_Expr_None}, none_expr{} {}
    Ast_Expr(Unary_Expr unary_expr) : kind{Ast_Expr_Unary}, unary_expr{unary_expr} {}
    Ast_Expr(Binary_Expr binary_expr) : kind{Ast_Expr_Binary}, binary_expr{binary_expr} {}
    Ast_Expr(Nested_Expr nested_expr) : kind{Ast_Expr_Nested}, nested_expr{nested_expr} {}
    Ast_Expr(Scope_Expr scope_expr) : kind{Ast_Expr_Scope}, scope_expr{scope_expr} {}
    Ast_Expr(If_Expr if_expr) : kind{Ast_Expr_If}, if_expr{if_expr} {}
    Ast_Expr(Return_Expr return_expr) : kind{Ast_Expr_Return}, return_expr{return_expr} {}
    Ast_Expr(Id_Expr id_expr) : kind{Ast_Expr_Id}, id_expr{id_expr} {}
    Ast_Expr(Def_Expr def_expr) : kind{Ast_Expr_Def}, def_expr{def_expr} {}
    Ast_Expr(Char_Expr char_expr) : kind{Ast_Expr_Char}, char_expr{char_expr} {}
    Ast_Expr(Str_Expr str_expr) : kind{Ast_Expr_Str}, str_expr{str_expr} {}
    Ast_Expr(Int_Expr int_expr) : kind{Ast_Expr_Int}, int_expr{int_expr} {}
    Ast_Expr(Float_Expr float_expr) : kind{Ast_Expr_Float}, float_expr{float_expr} {}
    Ast_Expr(Proc_Expr proc_expr) : kind{Ast_Expr_Proc}, proc_expr{proc_expr} {}
    Ast_Expr(Argument_Expr argument_expr) : kind{Ast_Expr_Argument}, argument_expr{argument_expr} {}
    Ast_Expr(Invoke_Expr invoke_expr) : kind{Ast_Expr_Invoke}, invoke_expr{invoke_expr} {}
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
    case Ast_Expr_Proc:
        return "Ast_Expr_Proc";
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
