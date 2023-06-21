#ifndef BEE_TOKEN_HPP
#define BEE_TOKEN_HPP

#include "bitset.hpp"
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

enum Token_Type : u64
{
    Token_None = 0,

    Token_NewLine = bitset(0),
    Token_Blank = bitset(1),
    Token_Comment = bitset(2),
    Token_Eof = bitset(3),

    Token_Struct = bitset(4),
    Token_Enum = bitset(5),
    Token_Union = bitset(6),
    Token_Break = bitset(7),
    Token_Case = bitset(8),
    Token_Continue = bitset(9),
    Token_Else = bitset(10),
    Token_For = bitset(11),
    Token_If = bitset(12),
    Token_Return = bitset(13),
    Token_Switch = bitset(14),
    Token_In = bitset(15),
    Token_And = bitset(16),
    Token_Or = bitset(17),

    Token_Id = bitset(18),

    Token_Float = bitset(19),
    Token_Int_Dec = bitset(20),
    Token_Int_Bin = bitset(21),
    Token_Int_Hex = bitset(22),
    Token_Str = bitset(23),
    Token_Raw_Str = bitset(24),
    Token_Char = bitset(25),

    Token_Increment = bitset(26),
    Token_Decrement = bitset(27),
    Token_Nested_Begin = bitset(28),
    Token_Nested_End = bitset(29),
    Token_Scope_Begin = bitset(30),
    Token_Scope_End = bitset(31),
    Token_Crochet_Begin = bitset(32),
    Token_Crochet_End = bitset(33),
    Token_Declare = bitset(34),
    Token_Define = bitset(35),
    Token_Assign = bitset(36),
    Token_Arrow = bitset(37),
    Token_Not = bitset(38),
    Token_Add = bitset(39),
    Token_Sub = bitset(40),
    Token_Mul = bitset(41),
    Token_Div = bitset(42),
    Token_Mod = bitset(43),
    Token_Bin_Not = bitset(44),
    Token_Bin_And = bitset(45),
    Token_Bin_Or = bitset(46),
    Token_Bin_Xor = bitset(47),
    Token_Shift_L = bitset(48),
    Token_Shift_R = bitset(49),
    Token_Eq = bitset(50),
    Token_Not_Eq = bitset(51),
    Token_Less = bitset(52),
    Token_Greater = bitset(53),
    Token_Less_Eq = bitset(54),
    Token_Greater_Eq = bitset(55),
    Token_Ref = bitset(56),
    Token_Dot = bitset(57),
    Token_Comma = bitset(58),
    Token_Semicolon = bitset(59),

    Token_Arithmetic = Token_Add | Token_Sub | Token_Mul | Token_Div | Token_Mod | Token_Bin_Not | Token_Bin_And |
                       Token_Bin_Or | Token_Bin_Xor | Token_Shift_L | Token_Shift_R,
    Token_Logic =
        Token_And | Token_Or | Token_Eq | Token_Not_Eq | Token_Less | Token_Less_Eq | Token_Greater | Token_Greater_Eq,

    Token_Type_Max = bitset(60),
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
        {Token_In, "'in' /!a"},
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

        {Token_Mul, "'*'"},
        {Token_Div, "'/'"},
        {Token_Mod, "'%'"},
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
    case Token_In:
        return "in";
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
