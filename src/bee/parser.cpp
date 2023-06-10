#include "parser.hpp"
#include "entity.hpp"
#include "escape_sequence.hpp"
#include "type_system.hpp"
#include "var.hpp"
#include <charconv>
#include <limits>

namespace bee
{

Parser::Parser(Scanner &scanner) : scanner{scanner} {}

void Parser::parse()
{
    Ast_Frame *main = ast.stack_push();
    type_system.def_atom_types(&ast);

    while (!eof())
    {
        parse_expr({Token_NewLine, Token_Eof});
    }
}

std::vector<Ast_Expr *> Parser::parse_compound(Token_Types sep_types, Token_Types end_types)
{
    std::vector<Ast_Expr *> compound;
    Ast_Expr *expr = NULL;
    Token end;

    while (!(end = scan(end_types)).ok and !eof())
    {
        if (Ast_Expr *expr = parse_expr(sep_types))
            compound.push_back(expr);
    }

    if (!end.ok)
    {
        throw error_expected(end, end_types);
    }

    return compound;
}

Ast_Expr *Parser::parse_expr(Token_Types end_types)
{
    Ast_Expr *head = NULL;
    Token end;

    while (!(end = scan(end_types)).ok and !eof())
    {
        if (Ast_Expr *expr = parse_expected_expr(head, end_types))
            head = expr;
    }

    if (!end.ok)
    {
        throw error_expected(end, end_types);
    }

    return head;
}

Ast_Expr *Parser::parse_expected_expr(Ast_Expr *prev, Token_Types end_types)
{
    // '+' / '-' Disambiguation, Acts as a sign if there is no previous expression
    if (!prev)
    {
        if (Token sign = scan({Token_Add, Token_Sub}); sign.ok)
        {
            Ast_Expr *expr = parse_expected_expr(NULL, end_types);
            return ast.expr_push<Unary_Expr>(sign, Prev_Expr, expr);
        }
    }

    Token token = scan({
        Token_Id,      Token_Char,    Token_Str,        Token_Int_Bin,     Token_Int_Dec,      Token_Int_Hex,
        Token_Float,   Token_Assign,  Token_And,        Token_Or,          Token_Add,          Token_Sub,
        Token_Mul,     Token_Div,     Token_Mod,        Token_Bin_Not,     Token_Bin_And,      Token_Bin_Or,
        Token_Bin_Xor, Token_Shift_L, Token_Shift_R,    Token_Eq,          Token_Not_Eq,       Token_Less,
        Token_Less_Eq, Token_Greater, Token_Greater_Eq, Token_Scope_Begin, Token_Parent_Begin, Token_Define,
        Token_Declare,
    });

    switch (token.type)
    {
    case Token_Scope_Begin: {
        ast.stack_push();
        auto body = parse_compound({Token_NewLine}, {Token_Scope_End});
        ast.stack_pop();
        Scope_Expr scope{.body = {body.begin(), body.end()}};
        return ast.expr_push<Scope_Expr>(scope);
    }

    case Token_Id: {
        Ast_Entity *entity = ast.frame->find(token.expr);
        Ast_Expr *ast_expr = ast.expr_push<Id_Expr>(token, entity);

        if (Token op = scan({Token_Define, Token_Declare}); op.ok)
        {
            return parse_def(ast_expr, op, end_types);
        }

        return ast.expr_push<Id_Expr>(token, entity);
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

        return ast.expr_push<Char_Expr>(data[0]);
    }

    case Token_Str: {
        std::string_view body = token.expr.substr(0, token.expr.size() - 1);
        std::string data = un_escape_string(body);
        return ast.expr_push<Str_Expr>(data);
    }

    case Token_Int_Bin:
    case Token_Int_Dec:
    case Token_Int_Hex: {
        u64 data;
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

        u32 size = data > std::numeric_limits<u32>::max() ? 8 : 4;
        return ast.expr_push<Int_Expr>(data, size);
    }

    case Token_Float: {
        f64 data;
        std::from_chars_result error = std::from_chars(token.expr.begin(), token.expr.end(), data);
        if (error.ec != std::errc{})
        {
            throw errorf(token, "bad float constant");
        }

        u32 size = data > std::numeric_limits<f32>::max() ? (u32)8 : (u32)4;
        return ast.expr_push<Float_Expr>(data, size);
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
            throw errorf(token, "missing prev-operand for binary expression");
        }
        Ast_Expr *post = parse_expected_expr(NULL, end_types);
        if (!post)
        {
            throw errorf(token, "missing post-operand for binary expression");
        }
        Ast_Entity *type_prev = type_system.type_expr(prev);
        Ast_Entity *type_post = type_system.type_expr(prev);
        u32 cast = type_system.type_cast(type_prev, type_post);

        if (cast < Type_Cast_Transmuted and type_prev->kind & Ast_Entity_Atom)
        {
            return ast.expr_push<Binary_Expr>(token, prev, post);
        }
        else
        {
            std::string_view type_prev_name = ast_entity_kind_name(type_prev->kind);
            std::string_view type_post_name = ast_entity_kind_name(type_post->kind);
            throw errorf(token, "cannot perform binary expression: <{}> {} <{}>", type_prev_name, token.expr,
                         type_post_name);
        }
    }

    default:
        return NULL;
    }
}

Ast_Expr *Parser::parse_def(Ast_Expr *prev, Token op, Token_Types end_types)
{
    if (!prev or prev->kind != Ast_Expr_Id)
    {
        throw errorf(op, "expected identifier for definition");
    }

    Id_Expr *id = &prev->id_expr;
    if (id->entity != NULL)
    {
        throw errorf(op, "redefinition of identifier '{:s}'", id->name.expr);
    }

    Ast_Expr *ast_expr = ast.expr_push<Def_Expr>();
    Def_Expr *def_expr = &ast_expr->def_expr;
    Ast_Entity *type = NULL;
    def_expr->op = op;
    def_expr->name = id->name;

    if (peek().type != Token_Assign)
    {
        Ast_Expr *type_expr = parse_expected_expr(NULL, end_types);
        Id_Expr *type_id = &type_expr->id_expr;
        if (type_expr->kind != Ast_Expr_Id or type_id->entity->kind & ~Ast_Entity_Type)
        {
            throw errorf(type_id->name, "does not name a type");
        }
        type = type_id->entity;
    }
    if (scan({Token_Assign}).ok)
    {
        def_expr->expr = parse_expr(end_types);
    }

    // Deduce type from expression type
    if (!type)
    {
        if (!def_expr->expr)
            throw errorf(op, "expected expression during definition");
        type = type_system.type_expr(def_expr->expr);
    }

    // TODO! procedures
    def_expr->entity = ast.entity_push<Var>(id->name.expr, type);
    return ast_expr;
}

bool Parser::eof() const
{
    return token_queue.empty() and scanner.eof();
}

Token Parser::peek()
{
    if (token_queue.size() > 0)
        return token_queue.front();
    return token_queue.emplace_back(scanner.tokenize());
}

Token Parser::scan(Token_Types types)
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

    if (!token.ok and token.type != Token_Eof)
    {
        token_queue.emplace_back(token);
    }
    return token;
}

Error Parser::error_expected(Token token, Token_Types types) const
{
    Stream stream;

    stream.print("expected ");
    for (Token_Type type : types)
    {
        stream.print("'{:s}', ", token_typename(type));
    }
    stream.print("got '{:s}'", token_typename(token.type));

    return bee_errorf("parser error", scanner.src, token, "{:s}", stream.str());
}

} // namespace bee
