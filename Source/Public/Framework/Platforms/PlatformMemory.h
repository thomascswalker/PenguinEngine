#pragma once

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

#include "Math/MathFwd.h"

struct PPlatformMemory
{
	static void free(void* memory)
	{
#if _WIN32
		VirtualFree(memory, 0, MEM_RELEASE);
#endif
	}

	static void* alloc(const size_t size)
	{
#if _WIN32
		return VirtualAlloc(nullptr, size, MEM_COMMIT, PAGE_READWRITE);
#endif
	}

	static void* realloc(void* memory, const size_t size)
	{
		if (memory != nullptr)
		{
			free(memory);
		}
		return alloc(size);
	}

	template <typename T>
	static void fill(void* memory, const size_t size, T value)
	{
		T* ptr = static_cast<T*>(memory);
		for (size_t index = 0; index < size; index++)
		{
			*(ptr++) = value;
		}
	}
};
