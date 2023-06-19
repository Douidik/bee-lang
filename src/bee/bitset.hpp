#ifndef BEE_BITSET_HPP
#define BEE_BITSET_HPP

#include "core.hpp"

namespace bee
{
    
constexpr u64 bitset(u64 n)
{
    return u64(1) << n;
}

} // namespace bee

#endif
