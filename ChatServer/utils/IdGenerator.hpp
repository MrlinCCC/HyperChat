#pragma once
#include <atomic>

template <typename T>
inline uint32_t generateAutoIncrementId()
{
    static std::atomic<uint32_t> counter{1};
    return counter.fetch_add(1);
}