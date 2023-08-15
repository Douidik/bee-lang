#ifndef BEE_BYTECODE_OBJECT_HPP
#define BEE_BYTECODE_OBJECT_HPP

#include "bitset.hpp"
#include "core.hpp"
#include "fwd.hpp"
#include "register.hpp"

namespace bee
{

enum Bc_Object_Type
{
    Bc_Object_None = 0,
    Bc_Object_Var = bitset(0),
    Bc_Object_Imm = bitset(1),
    Bc_Object_Interrupt = bitset(2),
};

enum Bc_Address_Type
{
    Bc_Address_None = 0,
    Bc_Address_Register = bitset(1),
    Bc_Address_Stack = bitset(2),
    Bc_Address_Heap = bitset(3),
};

// enum Bc_Interrupt
// {
//     Bc_Interrupt_None = 0,
//     Bc_Return = bitset(1),
//     Bc_ = bitset(1),
//     Bc_Return = bitset(1),
//     Bc_Exit = bitset(),
// };

struct Bc_Address
{
    Bc_Address_Type type;
    union {
        Register *reg;
        u64 x;
    };
};

struct Bc_Imm
{
    Ast_Entity *type;
};

struct Bc_Object
{
    Bc_Object_Type type;
    Bc_Address address;
    union {
        Var *var;
        Bc_Imm *imm;
    };
};

} // namespace bee

#endif
