#ifndef BEE_FRAME_HPP
#define BEE_FRAME_HPP

#include "core.hpp"
#include "entity.hpp"
#include "function.hpp"
#include <fmt/format.h>
#include <unordered_map>

namespace bee
{

struct Frame
{
    std::unordered_map<std::string_view, Ast_Entity *> defs;
    std::unordered_map<Ast_Entity *, u8 *> refs;
    std::unordered_map<Function *, Scope_Expr *> binds;
    Frame *owner;
    u32 depth;

    ~Frame();
    Ast_Entity *find_def(std::string_view name);
    u8 *find_ref(Ast_Entity *entity);
    u8 *push_ref(Ast_Entity *entity, u8 *ref);
    Scope_Expr *push_function(Function *function, Scope_Expr *scope);
    Scope_Expr *find_function(Function *function);

    template <typename T>
    T *push_def(T *entity)
    {
        static_assert(std::is_base_of_v<Ast_Entity, std::decay_t<T>>, "type is not an entity");
        return (T *)(defs[entity->name] = entity);
    }

    Error errorf(std::string_view fmt, auto... args)
    {
        return Error{"frame error", fmt::format(fmt::runtime(fmt), args...)};
    }
};

} // namespace bee

#endif
