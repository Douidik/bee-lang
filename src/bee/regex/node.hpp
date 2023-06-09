#ifndef BEE_REGEX_NODE_HPP
#define BEE_REGEX_NODE_HPP

#include "arena.hpp"
#include "state.hpp"
#include <set>
#include <vector>

namespace bee::regex
{

using Node_Arena = Arena<struct Node, 32>;

struct Node
{
    struct Cmp
    {
        bool operator()(const Node *a, const Node *b) const;
    };
    using Set = std::set<Node *, Cmp>;

    State state;
    s32 index;
    Set edges;

    usize submit(std::string_view expr, usize n) const;

    Node *push(Node *node);
    Node *merge(Node *node);
    Node *concat(Node *node);
    void map(s32 base);
    Node *end();
    Node *max_edge() const;
    bool branch() const;

    Set &make_members(Set &set);
    Set members();
};

} // namespace bee::regex

#endif
