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

struct memory_arena
{
    size_t Size;
    int64 *Base;
    size_t Used;
    size_t MaxUsed;
};

inline void InitMemoryArena(memory_arena *Arena, size_t size, int64 *base)
{
    Arena->Size = size;
    Arena->Base = base;
    Arena->Used = 0;
    Arena->MaxUsed = 0;
}

inline void* PushStructToArena(memory_arena *Arena, size_t size)
{
    ASSERT(Arena->Used + size <= Arena->Size);

    Arena->Used += size;
    Arena->MaxUsed += size;
    void *Result = (void*)((int64)Arena->Base + Arena->Used);
    return Result;
}
