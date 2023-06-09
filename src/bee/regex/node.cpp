#include "node.hpp"

namespace bee::regex
{

bool Node::Cmp::operator()(const Node *a, const Node *b) const
{
    return a->index < b->index;
}

usize Node::submit(std::string_view expr, usize n) const
{
    usize match = state.submit(expr, n);

    if (match != npos)
    {
        if (!branch() and match >= expr.size())
            return match;

        for (const Node *edge : edges)
        {
            usize match_fwd = edge->submit(expr, match);
            if (match_fwd != npos)
                return match_fwd;
        }

        if (!branch())
            return match;
    }

    return npos;
}

Node *Node::push(Node *node)
{
    node->map(end()->index + 1);
    return *edges.insert(node).first;
}

Node *Node::merge(Node *node)
{
    node->map(end()->index + 1);
    return concat(node);
}

Node *Node::concat(Node *node)
{
    for (Node *member : members())
    {
        if (!member->branch())
            member->edges.insert(node);
    }
    return node;
}

void Node::map(s32 base)
{
    for (Node *member : members())
        member->index += base;
}

Node *Node::end()
{
    Node *end = this;

    for (Node *member : members())
        end = end->index > member->index ? end : member;

    return end;
}

Node *Node::max_edge() const
{
    return !edges.empty() ? *edges.rbegin() : NULL;
}

bool Node::branch() const
{
    return !edges.empty() and max_edge()->index > index;
}

Node::Set &Node::make_members(Set &set)
{
    set.insert(this);

    for (Node *edge : edges)
    {
        if (edge->index > index)
            edge->make_members(set);
    }

    return set;
}

Node::Set Node::members()
{
    Set set{};
    return make_members(set);
}

} // namespace bee::regex
