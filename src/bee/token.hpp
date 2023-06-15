#ifndef BEE_TOKEN_HPP
#define BEE_TOKEN_HPP

#include "core.hpp"
#include "regex/regex.hpp"
#include <span>

namespace bee
{

enum Token_Type : u64;
using Syntax_Map = std::span<const std::pair<Token_Type, Regex>>;

struct Token
{
    std::string_view expr;
    Token_Type type;
    bool ok;
};

constexpr u64 token_type_n(u64 n)
{
    return u64(1) << n;
}

enum Token_Type : u64
{
    Token_None = 0,

    Token_NewLine = token_type_n(0),
    Token_Blank = token_type_n(1),
    Token_Comment = token_type_n(2),
    Token_Eof = token_type_n(3),

    Token_Struct = token_type_n(4),
    Token_Enum = token_type_n(5),
    Token_Union = token_type_n(6),
    Token_Break = token_type_n(7),
    Token_Case = token_type_n(8),
    Token_Continue = token_type_n(9),
    Token_Else = token_type_n(10),
    Token_For = token_type_n(11),
    Token_If = token_type_n(12),
    Token_Return = token_type_n(13),
    Token_Switch = token_type_n(14),
    Token_While = token_type_n(15),
    Token_And = token_type_n(16),
    Token_Or = token_type_n(17),

    Token_Id = token_type_n(18),

    Token_Float = token_type_n(19),
    Token_Int_Dec = token_type_n(20),
    Token_Int_Bin = token_type_n(21),
    Token_Int_Hex = token_type_n(22),
    Token_Str = token_type_n(23),
    Token_Raw_Str = token_type_n(24),
    Token_Char = token_type_n(25),

    Token_Increment = token_type_n(26),
    Token_Decrement = token_type_n(27),
    Token_Nested_Begin = token_type_n(28),
    Token_Nested_End = token_type_n(29),
    Token_Scope_Begin = token_type_n(30),
    Token_Scope_End = token_type_n(31),
    Token_Crochet_Begin = token_type_n(32),
    Token_Crochet_End = token_type_n(33),
    Token_Declare = token_type_n(34),
    Token_Define = token_type_n(35),
    Token_Assign = token_type_n(36),
    Token_Arrow = token_type_n(37),
    Token_Not = token_type_n(38),
    Token_Add = token_type_n(39),
    Token_Sub = token_type_n(40),
    Token_Mul = token_type_n(41),
    Token_Div = token_type_n(42),
    Token_Mod = token_type_n(43),
    Token_Bin_Not = token_type_n(44),
    Token_Bin_And = token_type_n(45),
    Token_Bin_Or = token_type_n(46),
    Token_Bin_Xor = token_type_n(47),
    Token_Shift_L = token_type_n(48),
    Token_Shift_R = token_type_n(49),
    Token_Eq = token_type_n(50),
    Token_Not_Eq = token_type_n(51),
    Token_Less = token_type_n(52),
    Token_Greater = token_type_n(53),
    Token_Less_Eq = token_type_n(54),
    Token_Greater_Eq = token_type_n(55),
    Token_Ref = token_type_n(56),
    Token_Dot = token_type_n(57),
    Token_Comma = token_type_n(58),
    Token_Semicolon = token_type_n(59),

    Token_Type_Max = token_type_n(60),
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
        {Token_Nested_Begin, "'('"},
        {Token_Nested_End, "')'"},
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
    case Token_Blank:
        return "<Blank>";
    case Token_Eof:
        return "<Eof>";
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
    case Token_Nested_Begin:
        return "(";
    case Token_Nested_End:
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
