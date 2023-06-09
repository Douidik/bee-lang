#ifndef BEE_REGEX_PARSER_HPP
#define BEE_REGEX_PARSER_HPP

#include "core.hpp"
#include "error.hpp"
#include "state.hpp"
#include <fmt/format.h>
#include <vector>
#include "node.hpp"

namespace bee::regex
{

struct Parser
{
    std::string_view src;
    Node_Arena &arena;
    const char *token;
    std::vector<Node *> sequences;

    Parser(std::string_view src, Node_Arena &arena);
    Node *parse();

    Node *parse_new_token();
    std::string_view parse_subsequence();

    std::pair<Node *, Node *> parse_binary_op(char op);
    Node *parse_pre_op(char op);
    Node *parse_post_op(char op);

    Node *parse_set(std::string_view set);
    Node *parse_scope();
    Node *parse_any();
    Node *parse_str(char quote);
    Node *parse_sequence();
    Node *parse_dash();
    Node *parse_not();
    Node *parse_or();
    Node *parse_quest();
    Node *parse_star();
    Node *parse_plus();
    Node *parse_wave();
	
    Node *def(const State &&state, s32 index = 0);

    Error errorf(std::string_view fmt, auto... args) const
    {
        return Error{
            "regex error",
            fmt::format(R"(with {{
{}
{:>{}} {}
}})",
                        src, '^', token - src.begin() + 1, fmt::format(fmt::runtime(fmt), args...)),
        };
    }
};

} // namespace bee::regex

#endif
