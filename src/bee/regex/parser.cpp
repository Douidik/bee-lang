#include "parser.hpp"
#include "node.hpp"
#include "regex.hpp"
#include <algorithm>

namespace bee::regex
{

Parser::Parser(std::string_view src, Node_Arena &arena) : src{src}, arena{arena}, token{src.end()} {}

Node *Parser::parse()
{
    for (token = src.begin(); token < src.end(); token++)
    {
        Node *sequence = parse_new_token();
        if (sequence != NULL)
        {
            sequences.push_back(sequence);
        }
    }

    for (usize i = 1; i < sequences.size(); i++)
    {
        sequences[0]->merge(sequences[i]);
    }
    return sequences.empty() ? NULL : sequences[0];
}

Node *Parser::parse_new_token()
{
    if (token >= src.end())
        return NULL;

    switch (*token)
    {
    case ' ':
    case '\f':
    case '\n':
    case '\r':
    case '\t':
    case '\v':
        token++;
        return parse_new_token();

    case '_':
        return parse_set(" \v\b\f\t");
    case 'a':
        return parse_set("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
    case 'o':
        return parse_set("!#$%&()*+,-./:;<=>?@[\\]^`{|}~");
    case 'n':
        return parse_set("0123456789");
    case 'Q':
        return parse_set("\"");
    case 'q':
        return parse_set("'");

    case '[':
        return parse_scope();
    case '^':
        return parse_any();
    case '!':
        return parse_not();
    case '/':
        return parse_dash();
    case '\'':
        return parse_str('\'');
    case '`':
        return parse_str('`');
    case '{':
        return parse_sequence();
    case '|':
        return parse_or();
    case '?':
        return parse_quest();
    case '*':
        return parse_star();
    case '+':
        return parse_plus();
    case '~':
        return parse_wave();

    case '}':
        throw errorf("unmatched sequence brace, missing <{{> token");

    case ']':
        throw errorf("unmatched sequence brace, missing <[> token");

    default:
        throw errorf("unknown token in regex, none of [_aonQq^'`{{}}!|?*+~]");
    }
}

std::string_view Parser::parse_subsequence()
{
    s32 depth = 1;
    const char *begin = token + 1;
    const char *end = token + 1;

    while (depth > 0 and end < src.end())
    {
        switch (*end)
        {
        case '{':
            depth++;
            break;

        case '}':
            depth--;
            break;
        }
        end++;
    }

    if (depth > 0)
    {
        throw errorf("unmatched sequence brace, missing <}}> token");
    }

    token = end - 1;
    return std::string_view{begin, end - 1};
}

std::pair<Node *, Node *> Parser::parse_binary_op(char op)
{
    return std::make_pair(parse_pre_op(op), parse_post_op(op));
}

Node *Parser::parse_pre_op(char op)
{
    if (sequences.empty())
        throw errorf("missing pre-operand for <{:c}> operator", op);

    Node *sequence = sequences.back();
    sequences.pop_back();
    return sequence;
}

Node *Parser::parse_post_op(char op)
{
    token++;
    Node *sequence = parse_new_token();

    if (!sequence)
        throw errorf("missing post-operand for <{:c}> operator", op);
    return sequence;
}

Node *Parser::parse_set(std::string_view set)
{
    return def(State{.option = Regex_Set, .str = set}, 0);
}

Node *Parser::parse_scope()
{
    static const Regex range_format = "'[' ^ '-' ^ ']'";

    if (!range_format.match(token, src.end()))
        throw errorf("range does not the match the format '{:s}'", range_format.src);

    char a = token[1];
    char b = token[3];
    token = &token[4];

    return def(State{.option = Regex_Scope, .range = {a, b}}, 0);
}

Node *Parser::parse_any()
{
    return def(State{Regex_Any}, 0);
}

Node *Parser::parse_str(char quote)
{
    const char *begin = token + 1;
    const char *end = std::find(begin, src.end(), quote);

    if (end == src.end())
        throw errorf("unmatched string quote, missing ending <{:c}> token", quote);

    std::string_view str{begin, token = end};
    return def(State{.option = Regex_Str, .str = str}, 0);
}

Node *Parser::parse_sequence()
{
    Parser parser{parse_subsequence(), arena};
    return parser.parse();
}

Node *Parser::parse_dash()
{
    return def(State{.option = Regex_Dash, .sequence = parse_post_op('/')}, 0);
}

Node *Parser::parse_not()
{
    return def(State{.option = Regex_Not, .sequence = parse_post_op('!')}, 0);
}

// Control flow structures:
// a: 1st binary operand
// b: 2nd binary operand
// o: unary operand
// $: epsilon
// ^: any
// x: none
// >: edge

Node *Parser::parse_or()
{
    //   > a
    // $
    //   > b
    auto [a, b] = parse_binary_op('|');
    Node *sequence = def(State{Regex_Eps}, 0);
    sequence->push(a);
    sequence->push(b);

    return sequence;
}

Node *Parser::parse_quest()
{
    //   > o
    // $
    //   > '$
    Node *sequence = def(State{Regex_Eps}, 0);
    sequence->merge(parse_pre_op('?'));
    sequence->push(def(State{Regex_Eps}));

    return sequence;
}

Node *Parser::parse_star()
{
    //   > o > $
    // $
    //   > $'
    Node *sequence = def(State{Regex_Eps}, 0);
    sequence->merge(parse_pre_op('*'));
    sequence->concat(sequence);
    sequence->push(def(State{Regex_Eps}));

    return sequence;
}

Node *Parser::parse_plus()
{
    // $ > e > $
    Node *sequence = parse_pre_op('+');
    return sequence->concat(sequence);
}

Node *Parser::parse_wave()
{
    //   > b
    // $
    //   > a > $
    //       > x
    auto [a, b] = parse_binary_op('~');
    auto sequence = def(State{Regex_Eps}, 0);
    sequence->push(b);
    sequence->push(a)->concat(sequence);
    a->merge(def(State{Regex_None}));

    return sequence;
}

Node *Parser::def(const State &&state, s32 index)
{
    return &arena.push(Node{state, index, {}});
}

} // namespace bee::regex
