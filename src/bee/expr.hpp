#ifndef BEE_EXPR_HPP
#define BEE_EXPR_HPP

#include "stream.hpp"
#include "token.hpp"
#include "type.hpp"
#include "var.hpp"
#include <string>

namespace bee
{

enum Ast_Expr_Kind
{
    Ast_Expr_Unary,
    Ast_Expr_Binary,
    Ast_Expr_Nested,
    Ast_Expr_Scope,
    Ast_Expr_If,
    Ast_Expr_Id,
    Ast_Expr_Def,
    Ast_Expr_Char,
    Ast_Expr_Str,
    Ast_Expr_Int,
    Ast_Expr_Float,
    Ast_Expr_Proc,
    Ast_Expr_Invoke,
};

struct Ast_Expr
{
    virtual ~Ast_Expr(){};
    virtual Ast_Expr_Kind kind() const = 0;

    virtual Type type() const
    {
        return Type::make_void();
    }
};

enum Order_Expr : u32
{
    Prev_Expr,
    Post_Expr,
};

struct Unary_Expr : Ast_Expr
{
    Token op;
    Order_Expr order;
    Ast_Expr *expr;

    Unary_Expr(Token op, Order_Expr order, Ast_Expr *expr) : op{op}, order{order}, expr{expr} {}
    Ast_Dump_Stream &ast_dump(Ast_Dump_Stream &stream, s32 depth) const;
    Type type() const;

    Ast_Expr_Kind kind() const
    {
        return Ast_Expr_Unary;
    }
};

struct Binary_Expr : Ast_Expr
{
    Token op;
    Ast_Expr *prev;
    Ast_Expr *post;

    Binary_Expr(Token op, Ast_Expr *prev, Ast_Expr *post) : op{op}, prev{prev}, post{post} {}
    Ast_Dump_Stream &ast_dump(Ast_Dump_Stream &stream, s32 depth) const;
    Type type() const;

    Ast_Expr_Kind kind() const
    {
        return Ast_Expr_Binary;
    }
};

struct Nested_Expr : Ast_Expr
{
    Ast_Expr *expr;

    Nested_Expr(Ast_Expr *expr) : expr{expr} {}
    Ast_Dump_Stream &ast_dump(Ast_Dump_Stream &stream, s32 depth) const;
    Type type() const;

    Ast_Expr_Kind kind() const
    {
        return Ast_Expr_Nested;
    }
};

struct Scope_Expr : Ast_Expr
{
    std::vector<Ast_Expr *> body;

    Scope_Expr(std::vector<Ast_Expr *> body) : body{body} {}
    Ast_Dump_Stream &ast_dump(Ast_Dump_Stream &stream, s32 depth) const;

    Ast_Expr_Kind kind() const
    {
        return Ast_Expr_Scope;
    }
};

struct If_Expr : Ast_Expr
{
    Ast_Expr *cond;

    Ast_Expr_Kind kind() const
    {
        return Ast_Expr_If;
    }
};

struct Id_Expr : Ast_Expr
{
    Var *var;

    Id_Expr(Var *var) : var{var} {}
    Ast_Dump_Stream &ast_dump(Ast_Dump_Stream &stream, s32 depth) const;
    Type type() const;

    Ast_Expr_Kind kind() const
    {
        return Ast_Expr_Id;
    }
};

struct Def_Expr : Ast_Expr
{
    Ast_Entity *entity;
    Ast_Expr *expr;
    Type *given_type;
    Token op;

    Def_Expr(Ast_Entity *entity, Ast_Expr *expr, Type *given_type, Token op) :
        entity{entity},
        expr{expr},
        given_type{given_type},
        op{op}
    {
    }

    Ast_Dump_Stream &ast_dump(Ast_Dump_Stream &stream, s32 depth) const;

    Ast_Expr_Kind kind() const
    {
        return Ast_Expr_Def;
    }
};

struct Char_Expr : Ast_Expr
{
    char data;

    Char_Expr(char data) : data{data} {}
    Ast_Dump_Stream &ast_dump(Ast_Dump_Stream &stream, s32 depth) const;
    Type type() const;

    Ast_Expr_Kind kind() const
    {
        return Ast_Expr_Char;
    }
};

struct Str_Expr : Ast_Expr
{
    std::string data;

    Str_Expr(std::string data) : data{data} {}
    Ast_Dump_Stream &ast_dump(Ast_Dump_Stream &stream, s32 depth) const;

    Ast_Expr_Kind kind() const
    {
        return Ast_Expr_Str;
    }
};

struct Int_Expr : Ast_Expr
{
    u64 data;
    u32 size;

    Int_Expr(u64 data, u32 size) : data{data}, size{size} {}
    Ast_Dump_Stream &ast_dump(Ast_Dump_Stream &stream, s32 depth) const;
    Type type() const;

    Ast_Expr_Kind kind() const
    {
        return Ast_Expr_Int;
    }
};

struct Float_Expr : Ast_Expr
{
    f64 data;
    u32 size;

    Float_Expr(f64 data, u32 size) : data{data}, size{size} {}
    Ast_Dump_Stream &ast_dump(Ast_Dump_Stream &stream, s32 depth) const;

    Ast_Expr_Kind kind() const
    {
        return Ast_Expr_Float;
    }
};

struct Proc_Expr : Ast_Expr
{
    std::vector<Def_Expr *> args;
    Type *type;
    Scope_Expr *scope;

    Proc_Expr(std::vector<Def_Expr *> args, Type *type, Scope_Expr *scope) : args{args}, type{type}, scope{scope} {}
    Ast_Dump_Stream &ast_dump(Ast_Dump_Stream &stream, s32 depth) const;

    Ast_Expr_Kind kind() const
    {
        return Ast_Expr_Proc;
    }
};

struct Invoke_Expr : Ast_Expr
{
    std::vector<Ast_Expr *> args;
    Var *proc;

    Invoke_Expr(std::vector<Ast_Expr *> args, Var *proc) : args{args}, proc{proc} {}
    Ast_Dump_Stream &ast_dump(Ast_Dump_Stream &stream, s32 depth) const;

    Ast_Expr_Kind kind() const
    {
        return Ast_Expr_Invoke;
    }
};

constexpr std::string_view ast_expr_kind_name(Ast_Expr_Kind kind)
{
    switch (kind)
    {
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
    }
}

} // namespace bee

#endif
