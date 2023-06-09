#ifndef BEE_ARENA_HPP
#define BEE_ARENA_HPP

#include "core.hpp"
#include "error.hpp"
#include <fmt/format.h>
#include <span>

namespace bee
{

// NOTE! cannot iterate on trivially, the data isn't contiguous between arena's nodes

template <typename T, usize N>
struct Arena
{
    T buffer[N];
    usize size;

    Arena(T zero = T{}) : size{0}, buffer{zero} {}

    void merge(std::span<T> span)
    {
        for (T &x : span)
        {
            push(x);
        }
    }

    T &push(const T &&x)
    {
        if (size + 1 > N)
        {
            throw Error{"arena error", "cannot push(): capacity exceeded"};
        }
        return (buffer[size++] = x);
    }

    T *pop()
    {
        if (size < 1)
        {
            throw Error{"arena error", "cannot pop(): empty arena"};
        }
        size--;
        return back();
    }

    T *begin()
    {
        return &buffer[0];
    }

    T *end()
    {
        return &buffer[size];
    }

    T *front()
    {
        return size != 0 ? &buffer[0] : NULL;
    }

    T *back()
    {

        return size != 0 ? &buffer[size - 1] : NULL;
    }
};

} // namespace bee

#endif
