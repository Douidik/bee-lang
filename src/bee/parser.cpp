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
    Ast_Frame *main_frame = ast.stack_push();
    type_system.type_std_def(&ast);

    Scope_Expr *scope_expr = &ast.expr_push<Scope_Expr>()->scope_expr;
    Compound_Expr compound = parse_compound(Token_NewLine, Token_Eof);
    scope_expr->compound = ast.compound_push(std::move(compound));
}

std::vector<Ast_Expr *> Parser::parse_compound(u64 sep_types, u64 end_types)
{
    std::vector<Ast_Expr *> compound;
    Token end;

    while (!(end = scan(end_types)).ok and !eof())
    {
        if (Ast_Expr *expr = parse_one_expr(NULL, sep_types))
            compound.emplace_back(expr);
    }

    if (!end.ok)
    {
        throw error_expected(end, end_types);
    }

    return compound;
}

Ast_Expr *Parser::parse_expr(u64 end_types)
{
    Token end;
    Ast_Expr *head = NULL;

    while (!(end = scan(end_types)).ok and !eof())
    {
        if (Ast_Expr *expr = parse_one_expr(head, end_types))
            head = expr;
    }

    if (!end.ok)
    {
        throw error_expected(end, end_types);
    }

    return head;
}

Ast_Expr *Parser::parse_one_expr(Ast_Expr *prev, u64 end_types)
{
    // '+' / '-' Disambiguation, Acts as a sign if there is no previous expression
    if (!prev)
    {
        if (Token sign = scan(Token_Add | Token_Sub); sign.ok)
        {
            Ast_Expr *expr = parse_one_expr(NULL, end_types);
            return ast.expr_push<Unary_Expr>(sign, Prev_Expr, expr);
        }
    }

    Token token =
        scan(Token_Id | Token_Char | Token_Str | Token_Int_Bin | Token_Int_Dec | Token_Int_Hex | Token_Float |
             Token_Assign | Token_And | Token_Or | Token_Add | Token_Sub | Token_Mul | Token_Div | Token_Mod |
             Token_Bin_Not | Token_Bin_And | Token_Bin_Or | Token_Bin_Xor | Token_Shift_L | Token_Shift_R | Token_Eq |
             Token_Not_Eq | Token_Less | Token_Less_Eq | Token_Greater | Token_Greater_Eq | Token_Scope_Begin |
             Token_Nested_Begin | Token_Define | Token_Declare | Token_NewLine | Token_Return);

    if (!token.ok)
        return NULL;

    switch (token.type)
    {
    case Token_Scope_Begin: {
        return parse_scope(Token_NewLine, Token_Scope_End);
    }

    case Token_Nested_Begin: {
        if (prev != NULL)
        {
            return parse_invoke(prev, token);
        }
        ast.stack_push();
        Ast_Expr *expr = parse_expr(Token_Nested_End);

        if (!expr or expr->kind & Ast_Expr_Def)
        {
            expr = parse_proc(expr, end_types);
        }
        ast.stack_pop();

        return expr;
    }

    case Token_Id: {
        Ast_Entity *entity = ast.frame->find(token.expr);
        Ast_Expr *ast_expr = ast.expr_push<Id_Expr>(token, entity);

        if (Token def = scan(Token_Define | Token_Declare); def.ok)
            return parse_def(ast_expr, def, end_types);
        if (!entity)
            throw errorf(token, "use of unknown identifier");

        return ast.expr_push<Id_Expr>(token, entity);
    }

    case Token_Char: {
        std::string_view body = token.expr.substr(1, token.expr.size() - 1);
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
        std::string_view body = token.expr.substr(1, token.expr.size() - 1);
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

        u32 size = data > std::numeric_limits<f32>::max() ? 8 : 4;
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
        Ast_Expr *post = parse_one_expr(NULL, end_types);
        if (!post)
        {
            throw errorf(token, "missing post-operand for binary expression");
        }
        Ast_Entity *prev_type = type_system.type_expr(prev);
        Ast_Entity *post_type = type_system.type_expr(post);

        if ((prev_type->kind | post_type->kind) & ~Ast_Entity_Atom)
        {
            std::string_view type_prev_name = ast_entity_kind_name(prev_type->kind);
            std::string_view type_post_name = ast_entity_kind_name(post_type->kind);
            throw errorf(token, "cannot perform binary expression: <{}> {} <{}>", type_prev_name, token.expr,
                         type_post_name);
        }

        // Atom composition: Take the largest size and float > signed > raw
        Atom_Type prev_atom = prev_type->atom_type;
        Atom_Type post_atom = post_type->atom_type;
        u32 desc = prev_atom.desc | post_atom.desc;
        u32 size = std::max(prev_atom.size, post_atom.size);
        Ast_Entity *type = type_system.compose_atom(desc, size);

        return ast.expr_push<Binary_Expr>(token, type, prev, post);
    }

    case Token_Return: {
        Ast_Expr *expr = parse_expr(end_types);
        return ast.expr_push<Return_Expr>(expr);
    }

    default:
        return parse_one_expr(prev, end_types);
    }
}

Ast_Expr *Parser::parse_scope(u64 sep_types, u64 end_types)
{
    ast.stack_push();
    Compound_Expr compound = parse_compound(sep_types, end_types);
    ast.stack_pop();

    Ast_Expr *ast_expr = ast.expr_push<Scope_Expr>();
    ast_expr->scope_expr.compound = ast.compound_push(std::move(compound));
    return ast_expr;
}

Ast_Expr *Parser::parse_def(Ast_Expr *prev, Token op, u64 end_types)
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

    if (op.type & Token_Define)
    {
        if (!peek(Token_Assign).ok)
        {
            type = parse_type(peek(Token_Assign), end_types);
        }
        if (scan(Token_Assign).ok)
        {
            def_expr->expr = parse_expr(end_types);
        }
    }
    else
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

    if (Token comma = scan(Token_Comma); comma.ok)
    {
        def_expr->next = parse_one_expr(NULL, end_types);
        if (!def_expr or def_expr->next->kind != Ast_Expr_Def)
            throw errorf(comma, "expected new defintion after comma");
    }

    def_expr->entity = ast.frame->push<Var>(id->name.expr, type);
    return ast_expr;
}

Ast_Expr *Parser::parse_invoke(Ast_Expr *proc, Token token)
{
    Ast_Entity *signature_entity = type_system.type_expr(proc);
    if (signature_entity->kind != Ast_Entity_Signature)
    {
        throw errorf(token, "cannot invoke expression of type: {:s}", signature_entity->name);
    }

    Signature_Type *signature = &signature_entity->signature_type;
    Ast_Expr *args = parse_argument(signature, signature->params, token);
    return ast.expr_push<Invoke_Expr>(proc, args);
}

Ast_Expr *Parser::parse_argument(Signature_Type *signature, Ast_Expr *param, Token token)
{

    if (!param)
        return NULL;

    Def_Expr *param_def = &param->def_expr;
    Ast_Expr *expr = parse_expr(Token_Comma | Token_Nested_End);

    if (!expr)
    {
        throw errorf(token, "missing argument '{:s}'", param_def->name.expr);
    }

    Ast_Entity *expr_type = type_system.type_expr(expr);
    Ast_Entity *param_type = type_system.type_entity(param_def->entity);
    u32 cast = type_system.type_cast(expr_type, param_type);
    if (cast & Type_Cast_Error)
    {
        std::string_view expr_typename = expr_type->name;
        std::string_view param_typename = param_type->name;
        throw errorf(token, "cannot cast argument of type '{:s}' into '{:s}'", expr_typename, param_typename);
    }

    return ast.expr_push<Argument_Expr>(expr, parse_argument(signature, param_def->next, token));
}

Ast_Expr *Parser::parse_proc(Ast_Expr *params, u64 end_types)
{
    Ast_Entity *type = type_system.void_type;

    if (Token arrow = scan(Token_Arrow); arrow.ok)
    {
        type = parse_type(arrow, end_types);
    }

    Ast_Expr *scope = parse_one_expr(NULL, end_types);
    if (!scope)
    {
        throw errorf(scanner.dummy_token(Token_Eof),
                     "TODO! signature type reference (for procedure references, etc...)");
    }

    // TODO! Temporary 'frame->owner', the signature was defined in function frame
    Signature_Type signature{params, type};
    Ast_Entity *signature_entity = ast.frame->owner->push<Signature_Type>(signature.make_name(), signature);

    return ast.expr_push<Proc_Expr>(signature_entity, scope);
}

Ast_Entity *Parser::parse_type(Token token, u64 end_types)
{
    Ast_Expr *expr = parse_one_expr(NULL, end_types);
    if (!expr)
    {
        throw errorf(token, "expected type expression");
    }

    if (expr->kind != Ast_Expr_Id or expr->id_expr.entity->kind & ~Ast_Entity_Type)
    {
        throw errorf(token, "expression does not name a type");
    }
    return expr->id_expr.entity;
}

bool Parser::eof() const
{
    return token_queue.empty() and scanner.eof();
}

Token Parser::peek(u64 types)
{
    Token token;

    if (token_queue.size() > 0)
        token = token_queue.front();
    else
        token = token_queue.emplace_back(scanner.tokenize());

    token.ok = token.type & types;
    return token;
}

Token Parser::scan(u64 types)
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

    token.ok = token.type & types;

    if (!token.ok and token.type != Token_Eof)
    {
        token_queue.emplace_back(token);
    }

    return token;
}

Error Parser::error_expected(Token token, u64 types)
{
    Stream stream;

    stream.print("expected ");
    for (u64 type = 1; type != Token_Type_Max; type <<= 1)
    {
        if (type & types)
        {
            stream.print("'{:s}', ", token_typename(Token_Type{type}));
        }
    }
    stream.print("got '{:s}'", token_typename(Token_Type{token.type}));

    return bee_errorf("parser error", scanner.src, token, "{:s}", stream.str());
}

} // namespace bee
