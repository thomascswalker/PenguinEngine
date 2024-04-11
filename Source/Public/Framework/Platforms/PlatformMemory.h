#pragma once

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

#include "Framework/Core/Core.h"
#include "Math/MathFwd.h"

struct PPlatformMemory
{
    static void Free(void* Memory)
    {
#if _WIN32
        VirtualFree(Memory, 0, MEM_RELEASE);
#endif
    }

    static void* Alloc(const size_t Size)
    {
#if _WIN32
        return VirtualAlloc(nullptr, Size, MEM_COMMIT, PAGE_READWRITE);
#endif
    }

    static void* Realloc(void* Memory, const size_t Size)
    {
        if (Memory != nullptr)
        {
            Free(Memory);
        }
        return Alloc(Size);
    }

    template <typename T>
    static void Fill(void* Memory, const size_t Size, T Value)
    {
        T* Ptr = static_cast<T*>(Memory);
        for (int32 Index = 0; Index < Size; Index++)
        {
            *(Ptr++) = Value;
        }
    }
};
