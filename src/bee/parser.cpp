#include "parser.hpp"
#include "entity.hpp"
#include "escape_sequence.hpp"
#include "function.hpp"
#include "type_system.hpp"
#include "var.hpp"
#include <charconv>
#include <limits>

namespace bee
{

Parser::Parser(Scanner &scanner) : scanner{scanner} {}

void Parser::parse()
{
    ast.main_frame = ast.push_frame(new Ast_Frame{});
    ast.main_scope = ast.push_expr(Scope_Expr{});
    type_system.std_types(&ast);

    ast.main_scope->compound = parse_compound(Token_NewLine, Token_Eof);
}

Compound_Expr *Parser::parse_compound(u64 sep_types, u64 end_types)
{
    Compound_Expr *compound = ast.push_compound(Compound_Expr{});
    Ast_Expr *expr = NULL;
    Token end;

    while (!(end = peek(end_types)).ok and !eof())
    {
        expr = parse_one_expr(expr, sep_types);
        if (expr != NULL)
            compound->emplace_back(expr);
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
    Ast_Expr *expr = NULL;

    while (!(end = peek(end_types)).ok and !eof())
    {
        expr = parse_one_expr(expr, end_types);
    }

    if (!end.ok)
    {
        throw error_expected(end, end_types);
    }

    return expr;
}

Ast_Expr *Parser::parse_one_expr(Ast_Expr *prev, u64 end_types)
{
    // Prev-operand unary expression
    if (!prev)
    {
        if (Token sign = scan(Token_Add | Token_Sub); sign.ok)
        {
            Unary_Expr *unary_expr = ast.push_expr(Unary_Expr{});
            unary_expr->op = sign;
            unary_expr->order = Post_Expr;
            unary_expr->expr = parse_one_expr(NULL, end_types);
            unary_expr->repr = unary_expr->op | unary_expr->expr->repr;

            if (type_system.expr_type(unary_expr->expr)->kind() != Ast_Entity_Atom)
            {
                throw errorf(sign, "cannot sign a expression that does not reduce to an atom");
            }

            return unary_expr;
        }
    }

    Token token =
        scan(Token_Id | Token_Char | Token_Str | Token_Int_Bin | Token_Int_Dec | Token_Int_Hex | Token_Float |
             Token_Increment | Token_Decrement | Token_Assign | Token_And | Token_Or | Token_Add | Token_Sub |
             Token_Mul | Token_Div | Token_Mod | Token_Bin_Not | Token_Bin_And | Token_Bin_Or | Token_Bin_Xor |
             Token_Shift_L | Token_Shift_R | Token_Eq | Token_Not_Eq | Token_Less | Token_Less_Eq | Token_Greater |
             Token_Greater_Eq | Token_Scope_Begin | Token_Nested_Begin | Token_If | Token_For | Token_Define |
             Token_Declare | Token_NewLine | Token_Return | Token_Struct | Token_Enum);

    if (!token.ok)
        throw error_expected(token, end_types);

    switch (token.type)
    {
    case Token_Scope_Begin: {
        if (prev != NULL)
        {
            return parse_struct((Id_Expr *)prev);
        }

        Ast_Frame *frame = ast.push_frame(new Ast_Frame{});
        Scope_Expr *scope = parse_scope(frame, Token_NewLine, Token_Scope_End);
        ast.pop_frame();
        return scope;
    }

    case Token_Nested_Begin: {
        if (prev != NULL)
        {
            if (prev->kind() != Ast_Expr_Id)
            {
                throw errorf(token, "cannot invoke expression of kind '{:s}'", ast_expr_kind_name(prev->kind()));
            }
            return parse_invoke((Id_Expr *)prev, token);
        }
        ast.push_frame(new Ast_Frame{});
        Ast_Expr *expr = parse_expr(Token_Nested_End);
        scan(Token_Nested_End);

        if (!expr or expr->kind() & Ast_Expr_Var)
        {
            return parse_signature((Var_Expr *)expr, end_types);
        }
        else
        {
            ast.pop_frame();
            return expr;
        }
    }

    case Token_If:
        return parse_if(token, end_types);

    case Token_For:
        return parse_for(token, end_types);

    case Token_Id: {
        Id_Expr *expr = ast.push_expr(Id_Expr{});
        expr->name = token;
        expr->entity = ast.frame->find(token.expr);

        if (Token def = scan(Token_Define | Token_Declare); def.ok)
            return parse_def(expr, def, end_types);
        if (!expr->entity)
            throw errorf(token, "use of unknown identifier");

        return expr;
    }

    case Token_Char: {
        Char_Expr *expr = ast.push_expr(Char_Expr{});
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
        Str_Expr *expr = ast.push_expr(Str_Expr{});
        std::string_view body = token.expr.substr(1, token.expr.size() - 1);
        expr->data = un_escape_string(body);
        return expr;
    }

    case Token_Int_Bin:
    case Token_Int_Dec:
    case Token_Int_Hex: {
        Int_Expr *expr = ast.push_expr(Int_Expr{});
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
        Float_Expr *expr = ast.push_expr(Float_Expr{});
        std::from_chars_result error = std::from_chars(token.expr.begin(), token.expr.end(), expr->data);
        if (error.ec != std::errc{})
        {
            throw errorf(token, "cannot parse float constant");
        }

        expr->size = expr->data > std::numeric_limits<f32>::max() ? 8 : 4;
        return expr;
    }

    case Token_Increment:
    case Token_Decrement:
        return parse_increment(token, prev, end_types);

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
    case Token_Greater_Eq:
        return parse_binary_expr(prev, parse_expr(end_types), token);

    case Token_Return: {
        Function_Expr *function_expr = (Function_Expr *)stack_find(Ast_Expr_Function);
        if (!function_expr)
        {
            throw errorf(token, "cannot return outside of a function scope");
        }

        Function *function = function_expr->function;
        Return_Expr *return_expr = ast.push_expr(Return_Expr{});
        return_expr->expr = parse_expr(end_types);

        Ast_Entity *return_type = type_system.expr_type(return_expr->expr);
        if (type_system.cast_type(return_type, function->type) >= Type_Cast_Transmuted)
        {
            throw errorf(token, "cannot return '{}' expected '{}'", return_type->name, function->type->name);
        }

        return return_expr;
    }

    case Token_Struct:
    case Token_Enum:
        return parse_record(token, end_types);

    default: {
        if (token.type & end_types)
            return NULL;
        return parse_one_expr(prev, end_types);
    }
    }
}

Unary_Expr *Parser::parse_increment(Token op, Ast_Expr *prev, u64 end_types)
{
    Unary_Expr *unary = ast.push_expr(Unary_Expr{});
    unary->op = op;

    if (prev != NULL)
    {
        unary->order = Post_Expr;
        unary->expr = prev;
    }
    else
    {
        unary->order = Prev_Expr;
        unary->expr = parse_expr(end_types);
        if (!unary->expr)
        {
            throw errorf(op, "missing operand for '{}' increment operation", op.expr);
        }
    }

    return unary;
}

Ast_Expr *Parser::parse_condition(Token kw, Ast_Expr *expr)
{
    if (!expr)
    {
        throw errorf(kw, "expression expected");
    }

    Ast_Entity *type = type_system.expr_type(expr);
    if (type_system.cast_type(type, type_system.bool_type) >= Type_Cast_Transmuted)
    {
        throw errorf(kw, "cannot evaluate condition of type '{}' as a boolean expression", type->name);
    }

    return expr;
}

If_Expr *Parser::parse_if(Token kw, u64 end_types)
{
    If_Expr *expr = ast.push_expr(If_Expr{});
    expr->frame = ast.push_frame(new Ast_Frame{});
    expr->condition = parse_condition(kw, parse_expr(Token_Scope_Begin));

    if (!expr->condition)
    {
        throw errorf(kw, "TODO! implement 'switch-if' expressions");
    }

    stack.push_back(expr);
    Ast_Expr *scope_if = parse_one_expr(NULL, end_types);
    if (!scope_if or scope_if->kind() != Ast_Expr_Scope)
    {
        throw errorf(kw, "expected scope after 'if' expression");
    }

    Ast_Expr *scope_else = NULL;
    if (kw = scan(Token_Else); kw.ok)
    {
        scope_else = parse_one_expr(NULL, end_types);
        if (!scope_else or scope_else->kind() != Ast_Expr_Scope)
        {
            throw errorf(kw, "expected scope after 'else' expression");
        }
    }

    ast.pop_frame();
    stack.pop_back();
    expr->scope_if = (Scope_Expr *)scope_if;
    expr->scope_else = (Scope_Expr *)scope_else;

    return expr;
}

Ast_Expr *Parser::parse_for(Token kw, u64 end_types)
{
    Ast_Frame *frame = ast.push_frame(new Ast_Frame{});
    Ast_Expr *expr = parse_expr(Token_Semicolon | Token_Scope_Begin);

    // TODO!
    // if (expr->kind() & In_Expr)
    // parse ranged expression

    if (Token start_sep = scan(Token_Semicolon); start_sep.ok)
    {
        For_Expr *for_expr = ast.push_expr(For_Expr{});
        for_expr->frame = frame;
        for_expr->start = expr;
        for_expr->condition = parse_condition(start_sep, parse_expr(Token_Semicolon));

        if (Token condition_sep = scan(Token_Semicolon); !condition_sep.ok)
        {
            throw errorf(start_sep, "expected iteration expression");
        }

        for_expr->iteration = parse_expr(Token_Scope_Begin);

        Ast_Expr *scope = parse_one_expr(NULL, end_types);
        if (!scope or scope->kind() != Ast_Expr_Scope)
        {
            throw errorf(kw, "expected scope after 'for' expression");
        }
        for_expr->scope = (Scope_Expr *)scope;

        return ast.pop_frame(), for_expr;
    }

    if (Token token = peek(Token_Scope_Begin); !expr or token.ok)
    {
        For_While_Expr *for_expr = ast.push_expr(For_While_Expr{});

        if (!expr)
        {
            Int_Expr *true_expr = ast.push_expr(Int_Expr{});
            true_expr->data = 1;
            true_expr->size = 1;
            expr = true_expr;
        }

        for_expr->condition = parse_condition(kw, expr);
        for_expr->frame = frame;

        Ast_Expr *scope = parse_one_expr(NULL, end_types);
        if (!scope or scope->kind() != Ast_Expr_Scope)
        {
            throw errorf(kw, "expected scope after 'for' expression");
        }
        for_expr->scope = (Scope_Expr *)scope;

        return ast.pop_frame(), for_expr;
    }

    throw errorf(kw, "cannot parse 'for' expression");
    return NULL;
}

Scope_Expr *Parser::parse_scope(Ast_Frame *frame, u64 sep_types, u64 end_types)
{
    Scope_Expr *expr = ast.push_expr(Scope_Expr{});
    expr->frame = frame;
    expr->compound = parse_compound(sep_types, end_types);

    if (Token end = scan(end_types); !end.ok)
    {
        throw error_expected(end, end_types);
    }

    return expr;
}

Binary_Expr *Parser::parse_binary_expr(Ast_Expr *prev, Ast_Expr *post, Token op)
{
    Binary_Expr *expr = ast.push_expr(Binary_Expr{});
    expr->op = op;
    expr->prev = prev;
    expr->post = post;

    if (!expr->prev)
    {
        throw errorf(op, "missing prev operand for binary expression");
    }
    if (!expr->post)
    {
        throw errorf(op, "missing post operand for binary expression");
    }

    Ast_Entity *prev_type = type_system.expr_type(expr->prev);
    Ast_Entity *post_type = type_system.expr_type(expr->post);

    if (op.type & Token_Assign)
    {
        if (type_system.cast_type(post_type, prev_type) >= Type_Cast_Transmuted)
        {
            throw errorf(op, "cannot perform assignment with discordant expressions '{}' and '{}'", prev_type->name,
                         post_type->name);
        }
        expr->type = post_type;
    }
    else if (op.type & Token_Arithmetic)
    {
        // Atom composition: Take the largest size and float > signed > raw
        if ((prev_type->kind() | post_type->kind()) & ~Ast_Entity_Atom)
        {
            throw errorf(op, "cannot perform arithmetic expression: '{}' {} '{}'", prev_type->name, op.expr,
                         post_type->name);
        }

        Atom_Type *prev_atom = (Atom_Type *)prev_type;
        Atom_Type *post_atom = (Atom_Type *)post_type;
        u32 desc = prev_atom->desc | post_atom->desc;
        u32 size = std::max(prev_atom->size, post_atom->size);
        expr->type = type_system.compose_atom(desc, size);
    }
    else if (op.type & Token_Logic)
    {
        if ((prev_type->kind() | post_type->kind()) & ~Ast_Entity_Atom)
        {
            throw errorf(op, "cannot perform boolean expression: '{}' {} '{}'", prev_type->name, op.expr,
                         post_type->name);
        }

        expr->type = type_system.bool_type;
    }

    return expr;
}

Ast_Expr *Parser::parse_def(Id_Expr *id, Token op, u64 end_types)
{
    if (id->entity != NULL)
    {
        throw errorf(op, "redefinition of identifier '{:s}'", id->name.expr);
    }

    Ast_Entity *type = NULL;
    Ast_Expr *expr;

    if (op.type & Token_Define)
    {
        if (!peek(Token_Assign).ok)
        {
            type = parse_type(peek(Token_Assign), end_types);
        }
        if (scan(Token_Assign).ok)
        {
            expr = parse_expr(end_types);
        }
    }
    else
    {
        expr = parse_expr(end_types);
    }

    if (expr != NULL and expr->kind() & Ast_Expr_Signature)
    {
        return parse_function(id, op, (Signature_Expr *)expr, end_types);
    }

    // Deduce type from expression type
    if (!type)
    {
        if (!expr)
            throw errorf(op, "expected expression or type during definition");
        type = type_system.expr_type(expr);
    }

    return parse_var(id, op, expr, type, end_types);
}

Var_Expr *Parser::parse_var(Id_Expr *id, Token op, Ast_Expr *expr, Ast_Entity *type, u64 end_types)
{
    Var_Expr *var_expr = ast.push_expr(Var_Expr{});
    var_expr->op = op;
    var_expr->name = id->name;

    var_expr->var = new Var{};
    var_expr->var->name = id->name.expr;
    var_expr->var->type = type;
    ast.frame->push(var_expr->var);

    if (Token comma = scan(Token_Comma); comma.ok)
    {
        var_expr->next = (Var_Expr *)parse_one_expr(NULL, end_types);
        if (!var_expr or var_expr->next->kind() != Ast_Expr_Var)
            throw errorf(comma, "expected new var expression intion after comma");
    }

    return var_expr;
}

Typedef_Expr *Parser::parse_typedef(Id_Expr *id, Token op, Record_Expr *record, u64 end_types)
{
    Typedef_Expr *typedef_expr = ast.push_expr(Typedef_Expr{});

    switch (record->kw.type)
    {
    case Token_Struct: {
        Struct_Type *type = ast.frame->push(new Struct_Type{});
        type->name = id->name.expr;
        type->frame = record->frame;
        typedef_expr->type = type;
    }

    case Token_Enum: {
        Enum_Type *type = ast.frame->push(new Enum_Type{});
        type->name = id->name.expr;
        type->frame = record->frame;
        typedef_expr->type = type;
    }

    default: {
        throw errorf(record->kw, "cannot typedef a non-record expression");
    }
    }

    return typedef_expr;
}

Function_Expr *Parser::parse_function(Id_Expr *id, Token op, Signature_Expr *signature, u64 end_types)
{
    Function *function = new Function{};
    function->params = signature->params;
    function->type = signature->type;
    function->name = id->name.expr;
    ast.frame->push(function);

    Function_Expr *function_expr = ast.push_expr(Function_Expr{});

    // Signature and function scope shares the same frame
    function_expr->function = function;
    Ast_Expr *scope = NULL;

    ast.push_frame(signature->frame);
    stack.push_back(function_expr);

    scope = parse_one_expr(NULL, 0);

    stack.pop_back();
    ast.pop_frame();

    if (!scope or scope->kind() != Ast_Expr_Scope)
    {
        throw errorf(id->name, "expected function scope after signature");
    }
    function_expr->scope = (Scope_Expr *)scope;

    return function_expr;
}

Invoke_Expr *Parser::parse_invoke(Id_Expr *id, Token token)
{
    if (id->entity->kind() != Ast_Entity_Function)
    {
        throw errorf(id->name, "cannot invoke expression of type: {:s}", id->name.expr);
    }

    Function *function = (Function *)id->entity;
    Invoke_Expr *invoke = ast.push_expr(Invoke_Expr{});
    invoke->function = function;
    invoke->args = parse_argument(function->params, token);
    return invoke;
}

Argument_Expr *Parser::parse_argument(Var_Expr *param, Token token)
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
    if (cast >= Type_Cast_Transmuted)
    {
        std::string_view expr_typename = expr_type->name;
        std::string_view param_typename = param_type->name;
        throw errorf(token, "cannot cast argument of type '{:s}' into '{:s}'", expr_typename, param_typename);
    }

    Argument_Expr *argument = ast.push_expr(Argument_Expr{});
    argument->expr = expr;

    Token sep = scan(Token_Comma | Token_Nested_End);
    if (sep.type & Token_Comma)
    {
        argument->next = parse_argument(param->next, token);
        if (!argument->next)
        {
            throw errorf(sep, "expected new argument after comma");
        }
    }
    if (sep.type & Token_Nested_End)
    {
        argument->next = NULL;
    }

    return argument;
}

Signature_Expr *Parser::parse_signature(Var_Expr *params, u64 end_types)
{
    Ast_Entity *type = type_system.void_type;

    if (Token arrow = scan(Token_Arrow); arrow.ok)
    {
        type = parse_type(arrow, end_types);
    }

    Signature_Expr *signature = ast.push_expr(Signature_Expr{});
    signature->params = params;
    signature->type = type;
    signature->frame = ast.pop_frame();

    return signature;
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

Record_Expr *Parser::parse_record(Token kw, u64 end_types)
{
    Record_Expr *record = ast.push_expr(Record_Expr{});
    record->kw = kw;
    record->frame = ast.push_frame(new Ast_Frame{});

    if (Token scope_begin = scan(Token_Scope_Begin); !scope_begin.ok)
    {
        throw errorf(scope_begin, "expected 'struct' body");
    }

    record->scope = parse_scope(record->frame, Token_NewLine, Token_Scope_End);
    for (Ast_Expr *expr : *record->scope->compound)
    {
        if (kw.type & Token_Struct and expr->kind() & ~(Ast_Expr_Var | Ast_Expr_Function))
        {
            std::string_view expr_name = ast_expr_kind_name(expr->kind());
            throw errorf(kw, "struct scope must only contain variable and function definitions got '{:s}'", expr_name);
        }

        if (kw.type & Token_Enum)
        {
            if (expr->kind() != Ast_Expr_Var)
            {
                std::string_view expr_name = ast_expr_kind_name(expr->kind());
                throw errorf(kw, "enum scope must only contain constant declaration got '{:s}'", expr_name);
            }

            if (Var_Expr *var = (Var_Expr *)expr; var->op.type != Token_Declare)
            {
                throw errorf(var->name, "enum member is not constant expression");
            }
        }
    }

    return record;
}

Struct_Expr *Parser::parse_struct(Ast_Expr *prev, Token scope_begin)
{
    if (prev->kind() != Ast_Expr_Id)
    {
        std::string_view got = ast_expr_kind_name(prev->kind());
        throw errorf(scope_begin, "expected type identifier before struct expression got '{:s}'", got);
    }

    Id_Expr *id = (Id_Expr *)prev;
    if (id->entity->kind() != Ast_Entity_Struct)
    {
        std::string_view got = ast_entity_kind_name(id->entity->kind());
        throw errorf(id->name, "identifier does not refer to a struct type, got '{:s}'", got);
    }

    Struct_Type *type = (Struct_Type *)id->entity;
    Struct_Expr *expr = ast.push_expr(Struct_Expr{});
    expr->type = type;
    expr->members = parse_member(type, 0);

    return expr;
}

Member_Expr *Parser::parse_member(Struct_Type *type, s32 n)
{
    
}

Ast_Expr *Parser::stack_find(Ast_Expr_Kind kind) const
{
    for (auto it = stack.rbegin(); it != stack.rend(); it++)
    {
        Ast_Expr *expr = *it;
        if (expr->kind() == kind)
            return expr;
    }
    return NULL;
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

    return bee_errorf("parser error", scanner.source, token, "{:s}", stream.str());
}

} // namespace bee
