#include "format.hpp"

namespace bee::regex
{

Format::Format(std::string_view name, Node *head) : name{name}, head{head}
{
    format_graph();
}

void Format::format_graph()
{
    print("strict digraph {{\n");

    if (head != NULL)
    {
        print(R"(rankdir=LR;bgcolor="#F9F9F9";compound=true{})", '\n');
        print(R"("{}" [shape="none"]{})", escape_string(name, 2), '\n');
        print(R"("{}" -> "{:p}" [label="{}"]{})", escape_string(name, 2), fmt::ptr(head), head->state, '\n');

        for (Node *node : head->members())
            format_node(node);
    }

    print("}}");
}

void Format::format_node(Node *node)
{
    switch (node->state.option)
    {
    case Regex_Not:
        return format_subgraph(node, R"(style=filled;bgcolor="#FBF3F3")");
    case Regex_Dash:
        return format_subgraph(node, R"(style=filled;bgcolor="#F4FDFF")");

    default:
        format_def(node);
        for (Node *edge : node->edges)
            format_cxn(node, edge);
    }
}

void Format::format_def(Node *node)
{
    auto shape = node->branch() ? "square" : "circle";
    print(R"("{:p}" [shape="{:s}", label="{:d}"]{})", fmt::ptr(node), shape, node->index, '\n');
}

void Format::format_cxn(Node *node, Node *edge)
{
    print(R"("{:p}" -> "{:p}" [label="{}"]{})", fmt::ptr(node), fmt::ptr(edge), edge->state, '\n');
}

void Format::format_subgraph(Node *node, std::string_view style)
{
    Node *sequence = node->state.sequence;

    print("subgraph cluster_{:p} {{\n", fmt::ptr(node));
    print("{:s}", style);
    format_def(node);
    format_cxn(node, sequence);

    for (Node *member : sequence->members())
    {
        format_node(member);
    }
    print("}}\n");

    Node *end = sequence->end();
    for (Node *edge : node->edges)
    {
        format_cxn(end, edge);
    }
}

} // namespace bee::regex
