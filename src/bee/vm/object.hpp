#ifndef BEE_VM_OBJECT_HPP
#define BEE_VM_OBJECT_HPP

#include "core.hpp"
#include <variant>

namespace bee
{
struct Ast_Entity;

enum Vm_Interrupt
{
    Vm_Interrupt_None,
    Vm_Return,
    Vm_Exit,
};

struct Vm_Object
{
    Ast_Entity *type;
    u8 *ref;
    
    Vm_Interrupt interrupt = Vm_Interrupt_None;
};

using Vm_Atom = std::variant<u8 *, u16 *, u32 *, u64 *, s8 *, s16 *, s32 *, s64 *, f32 *, f64 *>;

} // namespace bee

#endif
