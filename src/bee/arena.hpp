#ifndef BEE_ARENA_HPP
#define BEE_ARENA_HPP

#include "core.hpp"
#include "error.hpp"
#include <algorithm>
#include <deque>
#include <fmt/format.h>

namespace bee
{

template <typename T, usize N>
struct Arena
{
    T buffer[N];
    usize size;

    Arena(T zero = T{}) : size{0}, buffer{zero} {}

    Arena(auto begin, auto end) : size{(usize)std::distance(begin, end)}
    {
        if (size > N)
            throw error("cannot create arena from range: capacity exceeded");
        std::move(begin, end, buffer);
    }

    T &push(const T &&x)
    {
        if (size + 1 > N)
            throw error("cannot push(): capacity exceeded");
        return (buffer[size++] = x);
    }

    T *pop()
    {
        if (size < 1)
            throw error("cannot pop(): empty arena");
        size--;
        return back();
    }

    T *at(usize n)
    {
        return n < size ? &buffer[n] : NULL;
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

    Error error(std::string_view desc) const
    {
        return Error{"arena error", std::string(desc)};
    }
};

template <typename T, usize N>
struct Dyn_Arena
{
    std::deque<Arena<T, N>> chunks;
    usize size;

    Dyn_Arena() = default;

    Dyn_Arena(auto begin, auto end) : size{(usize)std::distance(begin, end)}
    {
        for (usize i = 0; i < size; i += N)
        {
            chunk_push() = Arena<T, N>{begin + i, begin + i + N};
        }
    }

    Dyn_Arena(auto container) : Dyn_Arena(container.begin(), container.end()) {}

    T *at(usize n)
    {
        usize chunk_n = n / N;
        usize item_n = n % N;

        if (chunk_n >= chunks.size() or item_n >= chunks[chunk_n].size)
            return NULL;
        return chunks[chunk_n].buffer[item_n];
    }

    T &push(const T &&x)
    {
        if (++size > chunks.size() * N)
            chunk_push();
        return chunk_back().push(std::move(x));
    }

    T *pop()
    {
        if (size < 1)
            throw error("cannot pop(): empty arena");
        size--;
        return chunk_back().pop();
    }

    T *front()
    {
        return chunk_front().front();
    }

    T *back()
    {
        return chunk_back().back();
    }

    Arena<T, N> &chunk_push()
    {
        return chunks.emplace_back();
    }

    Arena<T, N> &chunk_front()
    {
        if (chunks.empty())
            return chunk_push();
        return chunks.front();
    }

    Arena<T, N> &chunk_back()
    {
        if (chunks.empty())
            return chunk_push();
        return chunks.back();
    }

    Error error(std::string_view desc) const
    {
        return Error{"arena error", std::string(desc)};
    }
};

} // namespace bee

#endif
