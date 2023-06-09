#include "parser.hpp"
#include "entity.hpp"
#include "escape_sequence.hpp"
#include "var.hpp"
#include <charconv>
#include <limits>

namespace bee
{

Parser::Parser(Scanner &scanner) : scanner{scanner} {}

Ast Parser::parse()
{
    Frame *main = ast.stack_push();
    def_standard_types(main);

    while (!scanner.eof())
    {
        parse_expr(Token_NewLine);
    }
    return ast;
}

std::vector<Ast_Expr *> Parser::parse_compound(Token_Type sep_type, Token_Type end_type)
{
    std::vector<Ast_Expr *> compound;
    Ast_Expr *expr = NULL;
    Token end;

    while (!(end = scan({end_type, Token_Eof})).ok)
    {
        if (Ast_Expr *expr = parse_expr(sep_type))
            compound.push_back(expr);
    }

    if (!end.ok)
    {
        throw errorf(end, "expected '{:s}' got '{:s}'", token_typename(end_type), token_typename(end.type));
    }

    return compound;
}

Ast_Expr *Parser::parse_expr(Token_Type end_type)
{
    Ast_Expr *head = NULL;
    Token end;

    while (!(end = scan({end_type, Token_Eof})).ok)
    {
        if (Ast_Expr *expr = parse_expected_expr(head, end_type))
            head = expr;
    }

    if (end.type != end_type)
    {
        throw errorf(end, "expected '{:s}' got '{:s}'", token_typename(end_type), token_typename(end.type));
    }

    return head;

    // while (!scanner.eof())
    // {
    //     if (end = scan(end_type); end.ok)
    //     {
    //         return head;
    //     }

    //     if (Ast_Expr *expr = parse_expected_expr(head, end_type))
    //     {
    //         head = expr;
    //     }
    //     else
    //     {
    //         if (end = scan(end_type); !end.ok)
    //         {
    //             break; // goto error
    //         }
    //         return head;
    //     }
    // }
}

Ast_Expr *Parser::parse_expected_expr(Ast_Expr *prev, Token_Type end_type)
{
    // '+' / '-' Disambiguation, Acts as a sign if there is no previous expression
    if (!prev)
    {
        if (Token sign = scan({Token_Add, Token_Sub}); sign.ok)
        {
            Ast_Expr *expr = parse_expected_expr(NULL, end_type);
            return ast.expr_push(new Unary_Expr{sign, Prev_Expr, expr});
        }
    }

    Token token = scan({
        Token_Id,      Token_Char,    Token_Str,        Token_Int_Bin,     Token_Int_Dec,      Token_Int_Hex,
        Token_Float,   Token_Assign,  Token_And,        Token_Or,          Token_Add,          Token_Sub,
        Token_Mul,     Token_Div,     Token_Mod,        Token_Bin_Not,     Token_Bin_And,      Token_Bin_Or,
        Token_Bin_Xor, Token_Shift_L, Token_Shift_R,    Token_Eq,          Token_Not_Eq,       Token_Less,
        Token_Less_Eq, Token_Greater, Token_Greater_Eq, Token_Scope_Begin, Token_Parent_Begin,
    });

    switch (token.type)
    {
    case Token_Scope_Begin: {
        ast.stack_push();
        Scope_Expr *scope = new Scope_Expr({
            parse_compound(Token_NewLine, Token_Scope_End),
        });
        ast.stack_pop();
        return ast.expr_push(scope);
    }

    case Token_Id: {
        Ast_Entity *entity = ast.frame->find(token.expr);

        if (entity != NULL)
            return ast.expr_push(new Id_Expr{entity});

        Token def_token = scan({Token_Declare, Token_Define});
        if (def_token.ok)
        {
            Ast_Expr *expr = parse_expected_expr(NULL, end_type);
            if (!expr)
            {
                throw errorf(def_token, "expected expression after '{:s}'", token_typename(def_token.type));
            }

            // TODO! resolve underlying expression type
            return ast.expr_push(new Def_Expr{entity, expr, NULL, def_token});
        }

        throw errorf(token, "use of undeclared identifier");
    }

    case Token_Char: {
        std::string_view body = token.expr.substr(0, token.expr.size() - 1);
        std::string data = un_escape_string(body);
        if (data.size() < 1)
        {
            throw errorf(token, "empty character constant");
        }
        if (data.size() > 1)
        {
            throw errorf(token, "wide character constant");
        }

        return ast.expr_push(new Char_Expr{data.at(0)});
    }

    case Token_Str: {
        std::string_view body = token.expr.substr(0, token.expr.size() - 1);
        std::string data = un_escape_string(body);
        return ast.expr_push(new Str_Expr{data});
    }

    case Token_Int_Bin:
    case Token_Int_Dec:
    case Token_Int_Hex: {
        u64 data;
        u32 size;
        std::from_chars_result error;

        switch (token.type)
        {
        case Token_Int_Bin:
            error = std::from_chars<u64>(token.expr.begin(), token.expr.end(), data, 2);
            break;
        case Token_Int_Dec:
            error = std::from_chars<u64>(token.expr.begin(), token.expr.end(), data, 10);
            break;
        case Token_Int_Hex:
            error = std::from_chars<u64>(token.expr.begin(), token.expr.end(), data, 16);
            break;
        default:
            break;
        }

        if (error.ec != std::errc{})
        {
            throw errorf(token, "bad integer constant");
        }

        return ast.expr_push(new Int_Expr{
            data,
            data > std::numeric_limits<u32>::max() ? (u32)8 : (u32)4,
        });
    }

    case Token_Float: {
        f64 data;
        std::from_chars_result error = std::from_chars(token.expr.begin(), token.expr.end(), data);
        if (error.ec != std::errc{})
        {
            throw errorf(token, "bad float constant");
        }

        return ast.expr_push(new Float_Expr{
            data,
            data > std::numeric_limits<f32>::max() ? (u32)8 : (u32)4,
        });
    }

    case Token_Assign:
    case Token_And:
    case Token_Or:
    case Token_Add:
    case Token_Sub:
    case Token_Mul:
    case Token_Div:
    case Token_Mod:
    case Token_Bin_Not:
    case Token_Bin_And:
    case Token_Bin_Or:
    case Token_Bin_Xor:
    case Token_Shift_L:
    case Token_Shift_R:
    case Token_Eq:
    case Token_Not_Eq:
    case Token_Less:
    case Token_Less_Eq:
    case Token_Greater:
    case Token_Greater_Eq: {
        if (!prev)
        {
            throw errorf(token, "missing prev operand for binary expressione");
        }
        Ast_Expr *post = parse_expected_expr(NULL, end_type);
        if (!post)
        {
            throw errorf(token, "missing post operand for binary expressione");
        }
        // TODO! Check if operands 'does_cast()'
        return ast.expr_push(new Binary_Expr{token, prev, post});
    }

    default:
        return NULL;
    }
}

Token Parser::scan(Token_Type type)
{
    return scan({type});
}

Token Parser::scan(std::initializer_list<Token_Type> types)
{
    Token token;

    if (token_queue.size() > 0)
    {
        token = token_queue.front();
        token_queue.pop_front();
    }
    else
    {
        token = scanner.tokenize();
    }

    token.ok = false;
    for (Token_Type type : types)
    {
        token.ok |= token.type == type;
    }
    token.ok |= types.size() == 0;

    if (!token.ok)
    {
        token_queue.emplace_back(token);
    }
    return token;
}

void Parser::def_standard_types(Frame *frame)
{
    auto def_atom = [&](std::string_view name, Atom_Desc desc, u32 size) {
        Type *atom = new Type{Type::make_atom(desc, size)};
        frame->defs.emplace(name, atom);
    };

    def_atom("f16", Atom_Float, 2);
    def_atom("f32", Atom_Float, 4);
    def_atom("f64", Atom_Float, 8);

    def_atom("s8", Atom_Signed, 1);
    def_atom("s16", Atom_Signed, 2);
    def_atom("s32", Atom_Signed, 4);
    def_atom("s64", Atom_Signed, 8);

    def_atom("u8", Atom_Raw, 1);
    def_atom("u16", Atom_Raw, 2);
    def_atom("u32", Atom_Raw, 4);
    def_atom("u64", Atom_Raw, 8);

    def_atom("char", Atom_Signed, 1);
    def_atom("ssize", Atom_Signed, sizeof(usize));
    def_atom("usize", Atom_Raw, sizeof(usize));
}

} // namespace bee
