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
    ast.main_frame = ast.stack_push();
    ast.main_scope = ast.expr_push(Scope_Expr{});
    type_system.std_types(&ast);

    Compound_Expr compound = parse_compound(Token_NewLine, Token_Eof);
    ast.main_scope->compound = ast.compound_push(std::move(compound));
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
            Unary_Expr *unary_expr = ast.expr_push(Unary_Expr{});
            unary_expr->expr = parse_one_expr(NULL, end_types);
            unary_expr->order = Prev_Expr;
            unary_expr->op = sign;

            return unary_expr;
        }
    }

    Token token =
        scan(Token_Id | Token_Char | Token_Str | Token_Int_Bin | Token_Int_Dec | Token_Int_Hex | Token_Float |
             Token_Assign | Token_And | Token_Or | Token_Add | Token_Sub | Token_Mul | Token_Div | Token_Mod |
             Token_Bin_Not | Token_Bin_And | Token_Bin_Or | Token_Bin_Xor | Token_Shift_L | Token_Shift_R | Token_Eq |
             Token_Not_Eq | Token_Less | Token_Less_Eq | Token_Greater | Token_Greater_Eq | Token_Scope_Begin |
             Token_Nested_Begin | Token_Define | Token_Declare | Token_NewLine | Token_Return);

    if (!token.ok)
        throw error_expected(token, end_types);

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

        if (!expr or expr->kind() & Ast_Expr_Def)
        {
            expr = parse_function((Def_Expr *)expr, end_types);
        }
        ast.stack_pop();

        return expr;
    }

    case Token_Id: {
        Id_Expr *expr = ast.expr_push(Id_Expr{});
        expr->name = token;
        expr->entity = ast.frame->find(token.expr);

        if (Token def = scan(Token_Define | Token_Declare); def.ok)
            return parse_def(expr, def, end_types);
        if (!expr->entity)
            throw errorf(token, "use of unknown identifier");

        return expr;
    }

    case Token_Char: {
        Char_Expr *expr = ast.expr_push(Char_Expr{});
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

        expr->data = data[0];
        return expr;
    }

    case Token_Str: {
        Str_Expr *expr = ast.expr_push(Str_Expr{});
        std::string_view body = token.expr.substr(1, token.expr.size() - 1);
        expr->data = un_escape_string(body);
        return expr;
    }

    case Token_Int_Bin:
    case Token_Int_Dec:
    case Token_Int_Hex: {
        Int_Expr *expr = ast.expr_push(Int_Expr{});
        std::from_chars_result error;

        switch (token.type)
        {
        case Token_Int_Bin:
            error = std::from_chars<u64>(token.expr.begin(), token.expr.end(), expr->data, 2);
            break;
        case Token_Int_Dec:
            error = std::from_chars<u64>(token.expr.begin(), token.expr.end(), expr->data, 10);
            break;
        case Token_Int_Hex:
            error = std::from_chars<u64>(token.expr.begin(), token.expr.end(), expr->data, 16);
            break;
        default:
            break;
        }

        if (error.ec != std::errc{})
        {
            throw errorf(token, "cannot parse integer constant");
        }

        expr->size = expr->data > std::numeric_limits<u32>::max() ? 8 : 4;
        return expr;
    }

    case Token_Float: {
        Float_Expr *expr = ast.expr_push(Float_Expr{});
        std::from_chars_result error = std::from_chars(token.expr.begin(), token.expr.end(), expr->data);
        if (error.ec != std::errc{})
        {
            throw errorf(token, "cannot parse float constant");
        }

        expr->size = expr->data > std::numeric_limits<f32>::max() ? 8 : 4;
        return expr;
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
        Binary_Expr *expr = ast.expr_push(Binary_Expr{});
        expr->op = token;
        expr->prev = prev;
        expr->post = parse_one_expr(NULL, end_types);

        if (!expr->prev)
        {
            throw errorf(token, "missing prev operand for binary expression");
        }
        if (!expr->post)
        {
            throw errorf(token, "missing post operand for binary expression");
        }
        Ast_Entity *prev_type = type_system.expr_type(expr->prev);
        Ast_Entity *post_type = type_system.expr_type(expr->post);

        if ((prev_type->kind() | post_type->kind()) & ~Ast_Entity_Atom)
        {
            std::string_view type_prev_name = ast_entity_kind_name(prev_type->kind());
            std::string_view type_post_name = ast_entity_kind_name(post_type->kind());
            throw errorf(token, "cannot perform binary expression: <{}> {} <{}>", type_prev_name, token.expr,
                         type_post_name);
        }

        // Atom composition: Take the largest size and float > signed > raw
        Atom_Type *prev_atom = (Atom_Type *)prev_type;
        Atom_Type *post_atom = (Atom_Type *)post_type;
        u32 desc = prev_atom->desc | post_atom->desc;
        u32 size = std::max(prev_atom->size, post_atom->size);
        expr->type = type_system.compose_atom(desc, size);

        return expr;
    }

    case Token_Return: {
        Return_Expr *return_expr = ast.expr_push(Return_Expr{});
        return_expr->expr = parse_expr(end_types);
        return return_expr;
    }

    default:
        return parse_one_expr(prev, end_types);
    }
}

Scope_Expr *Parser::parse_scope(u64 sep_types, u64 end_types)
{
    ast.stack_push();
    Scope_Expr *expr = ast.expr_push(Scope_Expr{});
    Compound_Expr compound = parse_compound(sep_types, end_types);
    ast.stack_pop();

    expr->compound = ast.compound_push(std::move(compound));
    return expr;
}

Def_Expr *Parser::parse_def(Id_Expr *id, Token op, u64 end_types)
{
    if (id->entity != NULL)
    {
        throw errorf(op, "redefinition of identifier '{:s}'", id->name.expr);
    }

    Def_Expr *def = ast.expr_push(Def_Expr{});
    Ast_Entity *type = NULL;
    def->op = op;
    def->name = id->name;

    if (op.type & Token_Define)
    {
        if (!peek(Token_Assign).ok)
        {
            type = parse_type(peek(Token_Assign), end_types);
        }
        if (scan(Token_Assign).ok)
        {
            def->expr = parse_expr(end_types);
        }
    }
    else
    {
        def->expr = parse_expr(end_types);
    }

    // Deduce type from expression type
    if (!type)
    {
        if (!def->expr)
            throw errorf(op, "expected expression during definition");
        type = type_system.expr_type(def->expr);
    }

    if (Token comma = scan(Token_Comma); comma.ok)
    {
        def->next = (Def_Expr *)parse_one_expr(NULL, end_types);
        if (!def or def->next->kind() != Ast_Expr_Def)
            throw errorf(comma, "expected new defintion after comma");
    }

    def->var = new Var;
    def->var->name = id->name.expr;
    def->var->type = type;
    ast.frame->push(def->var);

    return def;
}

Invoke_Expr *Parser::parse_invoke(Ast_Expr *function, Token token)
{
    Ast_Entity *type = type_system.expr_type(function);
    if (type->kind() != Ast_Entity_Signature)
    {
        throw errorf(token, "cannot invoke expression of type: {:s}", type->name);
    }

    Signature *signature = (Signature *)type;
    Invoke_Expr *invoke = ast.expr_push(Invoke_Expr{});
    invoke->function = function;
    invoke->args = parse_argument(signature, signature->params, token);
    return invoke;
}

Argument_Expr *Parser::parse_argument(Signature *signature, Def_Expr *param, Token token)
{
    if (!param)
        return NULL;

    Ast_Expr *expr = parse_expr(Token_Comma | Token_Nested_End);

    if (!expr)
    {
        throw errorf(token, "missing argument '{:s}'", param->name.expr);
    }

    Ast_Entity *expr_type = type_system.expr_type(expr);
    Ast_Entity *param_type = type_system.entity_type(param->var);
    u32 cast = type_system.cast_type(expr_type, param_type);
    if (cast & Type_Cast_Error)
    {
        std::string_view expr_typename = expr_type->name;
        std::string_view param_typename = param_type->name;
        throw errorf(token, "cannot cast argument of type '{:s}' into '{:s}'", expr_typename, param_typename);
    }

    Argument_Expr *argument = ast.expr_push(Argument_Expr{});
    argument->expr = expr;
    argument->next = parse_argument(signature, param->next, token);

    return argument;
}

Function_Expr *Parser::parse_function(Def_Expr *params, u64 end_types)
{
    Ast_Entity *type = type_system.void_type;

    if (Token arrow = scan(Token_Arrow); arrow.ok)
    {
        type = parse_type(arrow, end_types);
    }

    Ast_Expr *scope = parse_one_expr(NULL, end_types);
    if (!scope or scope->kind() != Ast_Expr_Scope)
    {
        throw errorf(scanner.dummy_token(Token_Eof),
                     "TODO! signature type reference (for function references, etc...)");
    }

    // TODO! Temporary 'frame->owner', the signature was defined in function frame
    Signature *signature = new Signature;
    signature->type = type;
    signature->params = params;
    signature->name = signature->make_name();
    ast.frame->owner->push(signature);

    Function_Expr *function = ast.expr_push(Function_Expr{});
    function->signature = signature;
    function->scope = (Scope_Expr *)scope;
    return function;
}

Ast_Entity *Parser::parse_type(Token token, u64 end_types)
{
    Ast_Expr *expr = parse_one_expr(NULL, end_types);
    if (!expr)
    {
        throw errorf(token, "expected type expression");
    }

    Id_Expr *id = (Id_Expr *)expr;
    if (expr->kind() != Ast_Expr_Id or id->entity->kind() & ~Ast_Entity_Type)
    {
        throw errorf(token, "expression does not name a type");
    }
    return id->entity;
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
