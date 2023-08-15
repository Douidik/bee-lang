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
struct Frame;
struct Ast_Entity;
struct Var;
struct Function;
struct Struct_Type;
struct Enum_Type;

enum Ast_Expr_Kind : u32
{
    Ast_Expr_None = bitset(0),
    Ast_Expr_Unary = bitset(1),
    Ast_Expr_Binary = bitset(2),
    Ast_Expr_Nested = bitset(3),
    Ast_Expr_Scope = bitset(4),
    Ast_Expr_Return = bitset(5),
    Ast_Expr_Id = bitset(6),
    Ast_Expr_Var = bitset(7),
    Ast_Expr_Char = bitset(8),
    Ast_Expr_Str = bitset(9),
    Ast_Expr_Int = bitset(10),
    Ast_Expr_Float = bitset(11),
    Ast_Expr_Signature = bitset(12),
    Ast_Expr_Function = bitset(13),
    Ast_Expr_Argument = bitset(14),
    Ast_Expr_Invoke = bitset(15),
    Ast_Expr_If = bitset(16),
    Ast_Expr_For = bitset(17),
    Ast_Expr_For_Range = bitset(18),
    Ast_Expr_For_While = bitset(19),
    Ast_Expr_Typedef = bitset(20),
    Ast_Expr_Record = bitset(21),
    Ast_Expr_Struct = bitset(22),
    Ast_Expr_Member = bitset(23),

    Ast_Expr_Lit = Ast_Expr_Char | Ast_Expr_Str | Ast_Expr_Int | Ast_Expr_Float,
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

    Token repr;
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
    Frame *frame;
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

struct Var_Expr : Ast_Expr_Impl<Ast_Expr_Var>
{
    Var *var;
    Ast_Expr *expr;
    Token op;
    Token name;
    Var_Expr *next;
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

struct Signature_Expr : Ast_Expr_Impl<Ast_Expr_Signature>
{
    Var_Expr *params;
    Ast_Entity *type;
    Frame *frame;
};

struct Function_Expr : Ast_Expr_Impl<Ast_Expr_Function>
{
    Function *function;
    Scope_Expr *scope;
};

struct Argument_Expr : Ast_Expr_Impl<Ast_Expr_Argument>
{
    Ast_Expr *expr;
    Argument_Expr *next;
};

struct Invoke_Expr : Ast_Expr_Impl<Ast_Expr_Invoke>
{
    Function *function;
    Argument_Expr *args;
};

struct If_Expr : Ast_Expr_Impl<Ast_Expr_If>
{
    Ast_Expr *condition;
    Scope_Expr *scope_if;
    Scope_Expr *scope_else;
    Frame *frame;
};

// TODO!
// struct For_Range_Expr : Ast_Expr_Impl<Ast_Expr_For_Range>
// {
//     Ast_Expr *condition;
//     Scope_Expr *scope;
// };

struct For_Expr : Ast_Expr_Impl<Ast_Expr_For>
{
    Ast_Expr *start;
    Ast_Expr *condition;
    Ast_Expr *iteration;
    Scope_Expr *scope;
    Frame *frame;
};

struct For_While_Expr : Ast_Expr_Impl<Ast_Expr_For_While>
{
    Ast_Expr *condition;
    Scope_Expr *scope;
    Frame *frame;
};
struct Typedef_Expr : Ast_Expr_Impl<Ast_Expr_Typedef>
{
    Token op;
    Token name;
    Ast_Entity *type;
};

struct Record_Expr : Ast_Expr_Impl<Ast_Expr_Record>
{
    Token kw;
    Scope_Expr *scope;
    Frame *frame;
};
 
struct Member_Expr : Ast_Expr_Impl<Ast_Expr_Member>
{
    Id_Expr *id;
    Token op;
    Ast_Expr *expr;
    Member_Expr *next;
};

struct Struct_Expr : Ast_Expr_Impl<Ast_Expr_Struct>
{
    Struct_Type *type;
    Member_Expr *members;
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
    case Ast_Expr_Var:
        return "Ast_Expr_Var";
    case Ast_Expr_Char:
        return "Ast_Expr_Char";
    case Ast_Expr_Str:
        return "Ast_Expr_Str";
    case Ast_Expr_Int:
        return "Ast_Expr_Int";
    case Ast_Expr_Float:
        return "Ast_Expr_Float";
    case Ast_Expr_Signature:
        return "Ast_Expr_Signature";
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
