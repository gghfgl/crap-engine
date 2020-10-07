#pragma once

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef float float32;
typedef double float64;

#define ASSERT(x) if (!(x)) __debugbreak();

struct memory_pool
{
    size_t Size;
    int64 *Base;
    size_t Used;
    size_t MaxUsed;
};

// TODO: implement a decent mem pool
inline void InitMemoryPool(memory_pool *MemPool, size_t size, int64 *base)
{
    MemPool->Size = size;
    MemPool->Base = base;
    MemPool->Used = 0;
    MemPool->MaxUsed = 0;
}

inline void* PushStructToPool(memory_pool *MemPool, size_t size)
{
    ASSERT(MemPool->Used + size <= MemPool->Size);

    MemPool->Used += size;
    MemPool->MaxUsed += size;
    void *Result = (void*)((int64)MemPool->Base + MemPool->Used);
    return Result;
}

inline void RefreshMemoryPool(memory_pool *MemPool)
{
    MemPool->MaxUsed = 0;
}
