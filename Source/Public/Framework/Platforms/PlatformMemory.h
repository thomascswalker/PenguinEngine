#pragma once

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

/**
 * @brief Platform-specific memory management functions.
 */
namespace PlatformMemory
{
	/**
	 * @brief Frees the specified memory block.
	 * @param memory The memory block to free.
	 */
	static void free(void* memory)
	{
#if _WIN32
		VirtualFree(memory, 0, MEM_RELEASE);
#endif
	}

	/**
	 * @brief Allocates a new memory block with the specified size.
	 * @param size The size of the new memory block.
	 * @return The memory block which was allocated.
	 */
	template <typename T = void>
	static T* alloc(const size_t size)
	{
#if _WIN32
		return (T*)VirtualAlloc(nullptr, size, MEM_COMMIT, PAGE_READWRITE);
#endif
	}

	/**
	 * @brief Frees and then allocates the specified memory block.
	 * @param memory The memory block to free.
	 * @param size The size of the new memory block.
	 * @return The memory block which was reallocated.
	 */
	template <typename T = void>
	static T* realloc(void* memory, const size_t size)
	{
		if (memory != nullptr)
		{
			free(memory);
		}
		return alloc<T>(size);
	}

	/**
	 * @brief Fills the specified memory block to `size` with the specified `value` of type `T`.
	 * @tparam T The value type to fill with.
	 * @param memory The memory block to fill.
	 * @param size The size to fill up to.
	 * @param value The value to fill with.
	 */
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
