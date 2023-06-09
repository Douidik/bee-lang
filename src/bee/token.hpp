#ifndef BEE_TOKEN_HPP
#define BEE_TOKEN_HPP

#include "core.hpp"
#include "regex/regex.hpp"
#include <span>

namespace bee
{

enum Token_Type : u32;
using Syntax_Map = std::span<const std::pair<Token_Type, Regex>>;

struct Token
{
    std::string_view expr;
    Token_Type type;
    bool ok;
};

enum Token_Type : u32
{
    Token_None,

    Token_NewLine,
    Token_Empty,
    Token_Blank,
    Token_Eof,
    Token_Comment,

    Token_Struct,
    Token_Enum,
    Token_Union,
    Token_Break,
    Token_Case,
    Token_Continue,
    Token_Else,
    Token_For,
    Token_If,
    Token_Return,
    Token_Switch,
    Token_While,
    Token_And,
    Token_Or,

    Token_Id,

    Token_Float,
    Token_Int_Dec,
    Token_Int_Bin,
    Token_Int_Hex,
    Token_Str,
    Token_Raw_Str,
    Token_Char,

    Token_Increment,
    Token_Decrement,
    Token_Parent_Begin,
    Token_Parent_End,
    Token_Scope_Begin,
    Token_Scope_End,
    Token_Crochet_Begin,
    Token_Crochet_End,
    Token_Declare,
    Token_Define,
    Token_Assign,
    Token_Arrow,
    Token_Not,
    Token_Add,
    Token_Sub,
    Token_Mul,
    Token_Div,
    Token_Mod,
    Token_Bin_Not,
    Token_Bin_And,
    Token_Bin_Or,
    Token_Bin_Xor,
    Token_Shift_L,
    Token_Shift_R,
    Token_Eq,
    Token_Not_Eq,
    Token_Less,
    Token_Greater,
    Token_Less_Eq,
    Token_Greater_Eq,
    Token_Ref,
    Token_Dot,
    Token_Comma,
    Token_Semicolon,

    Token_Count,
};

static Syntax_Map bee_syntax_map()
{
    static const std::pair<Token_Type, Regex> map[] = {
        {Token_Blank, "_+"},
        {Token_Comment, "'//' {{{'\\'^}|^} ~ /'\n'}? /'\n'"},
	{Token_NewLine, "'\n'"},
	
        // Every token has the same regex pattern "'<name>' /!a"
        {Token_Enum, "'enum' /!a"},
        {Token_Union, "'union' /!a"},
        {Token_Struct, "'struct' /!a"},
        {Token_Break, "'break' /!a"},
        {Token_Case, "'case' /!a"},
        {Token_Continue, "'continue' /!a"},
        {Token_Else, "'else' /!a"},
        {Token_For, "'for' /!a"},
        {Token_If, "'if' /!a"},
        {Token_Return, "'return' /!a"},
        {Token_Switch, "'switch' /!a"},
        {Token_While, "'while' /!a"},
        {Token_And, "'and' /!a"},
        {Token_Or, "'or' /!a"},

        {Token_Scope_Begin, "'{'"},
        {Token_Scope_End, "'}'"},
        {Token_Parent_Begin, "'('"},
        {Token_Parent_End, "')'"},
        {Token_Crochet_Begin, "']'"},
        {Token_Crochet_End, "'['"},

        {Token_Arrow, "'->'"},
        {Token_Increment, "'++'"},
        {Token_Decrement, "'--'"},
        {Token_Add, "'+'"},
        {Token_Sub, "'-'"},

        {Token_Float, "{[0-9]+ '.' [0-9]*} |"
                      "{[0-9]* '.' [0-9]+}  "
                      "{'e'|'E' {'+'|'-'}? [0-9]+}?"},

        {Token_Int_Bin, "'0b'{[0-1]      }+"},
        {Token_Int_Hex, "'0x'{[0-9]|[a-f]}+"},
        {Token_Int_Dec, "    {[0-9]      }+"},

        {Token_Str, "Q {{{'\\'^}|^} ~ /{Q|'\n'}} ? {Q|'\n'}"},
        {Token_Char, "q {{{'\\'^}|^} ~ /{q|'\n'}} {q|'\n'}"},
        {Token_Id, "{a|'_'} {a|'_'|n}*"},

        {Token_Declare, "'::'"},
        {Token_Define, "':'"},

        {Token_Semicolon, "';'"},
        {Token_Comma, "','"},
        {Token_Dot, "'.'"},
        {Token_And, "'&&'"},
        {Token_Or, "'||'"},
        {Token_Bin_Not, "'~'"},
        {Token_Bin_Or, "'|'"},
        {Token_Bin_Xor, "'^'"},
        {Token_Shift_L, "'<<'"},
        {Token_Shift_R, "'>>'"},
        {Token_Div, "'/'"},
        {Token_Mod, "'%'"},
        {Token_Eq, "'=='"},
        {Token_Not_Eq, "'!='"},
        {Token_Less_Eq, "'<='"},
        {Token_Greater_Eq, "'>='"},
        {Token_Less, "'<'"},
        {Token_Greater, "'>'"},
        {Token_Not, "'!'"},
        {Token_Assign, "'='"},
        {Token_Ref, "'&'"},

        {Token_None, "^~/{_?}"},
    };

    return map;
}

constexpr std::string_view token_typename(Token_Type type)
{
    switch (type)
    {
    case Token_None:
        return "<None>";
    case Token_NewLine:
        return "<NewLine>";
    case Token_Empty:
        return "<Empty>";
    case Token_Blank:
        return "<Blank>";
    case Token_Eof:
        return "<Eof>";
    case Token_Comment:
        return "<Comment>";
    case Token_Struct:
        return "struct";
    case Token_Enum:
        return "enum";
    case Token_Union:
        return "union";
    case Token_Break:
        return "break";
    case Token_Case:
        return "case";
    case Token_Continue:
        return "continue";
    case Token_Else:
        return "else";
    case Token_For:
        return "for";
    case Token_If:
        return "if";
    case Token_Return:
        return "return";
    case Token_Switch:
        return "switch";
    case Token_While:
        return "while";
    case Token_And:
        return "and";
    case Token_Or:
        return "or";
    case Token_Id:
        return "<Id>";
    case Token_Float:
        return "<Float>";
    case Token_Int_Dec:
        return "<Int_Dec>";
    case Token_Int_Bin:
        return "<Int_Bin>";
    case Token_Int_Hex:
        return "<Int_Hex>";
    case Token_Str:
        return "<Str>";
    case Token_Raw_Str:
        return "<Raw_Str>";
    case Token_Char:
        return "<Char>";
    case Token_Increment:
        return "++";
    case Token_Decrement:
        return "--";
    case Token_Parent_Begin:
        return "(";
    case Token_Parent_End:
        return ")";
    case Token_Scope_Begin:
        return "{";
    case Token_Scope_End:
        return "}";
    case Token_Crochet_Begin:
        return "[";
    case Token_Crochet_End:
        return "]";
    case Token_Declare:
        return "::";
    case Token_Define:
        return ":";
    case Token_Arrow:
        return "->";
    case Token_Assign:
        return "=";
    case Token_Not:
        return "!";
    case Token_Add:
        return "+";
    case Token_Sub:
        return "-";
    case Token_Mul:
        return "*";
    case Token_Div:
        return "/";
    case Token_Mod:
        return "%";
    case Token_Bin_Not:
        return "~";
    case Token_Bin_And:
        return "&";
    case Token_Bin_Or:
        return "|";
    case Token_Bin_Xor:
        return "^";
    case Token_Shift_L:
        return "<<";
    case Token_Shift_R:
        return ">>";
    case Token_Eq:
        return "==";
    case Token_Not_Eq:
        return "!=";
    case Token_Less:
        return "<";
    case Token_Greater:
        return ">";
    case Token_Less_Eq:
        return "<=";
    case Token_Greater_Eq:
        return ">=";
    case Token_Ref:
        return "&";
    case Token_Dot:
        return ".";
    case Token_Comma:
        return ",";
    case Token_Semicolon:
        return ";";
    default:
        return "?";
    }
}

} // namespace bee

#endif
