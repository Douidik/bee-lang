#ifndef BEE_BYTECODE_INSTRUCTION_HPP
#define BEE_BYTECODE_INSTRUCTION_HPP

#include "core.hpp"
#include "object.hpp"

namespace bee
{

enum Bc_Opcode : u8
{
    Bc_None,

    Bc_Mov,
    Bc_Mov_Const,

    Bc_Invoke,
    Bc_Jump,

    Bc_Increment,
    Bc_Decrement,

    Bc_Add,
    Bc_Sub,
    Bc_Mul,
    Bc_Div,
    Bc_Mod,

    Bc_Eq,
    Bc_Not_Eq,
    Bc_Less,
    Bc_Less_Eq,
    Bc_Greater,
    Bc_Greater_Eq,

    Bc_Not,
    Bc_And,
    Bc_Or,

    Bc_Bin_Not,
    Bc_Bin_And,
    Bc_Bin_Or,
    Bc_Bin_Xor,
    Bc_Shift_L,
    Bc_Shift_R,
};

struct Bc_Instruction
{
    Bc_Opcode opcode;

    union {
        Bc_Object *v;
        struct
        {
            Bc_Object *x, y;
        };
	u64 n;
    };
};

} // namespace bee

#endif
