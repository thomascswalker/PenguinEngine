﻿#pragma once

#include <memory>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

#include "Core/Types.h"

constexpr uint32 g_bytesPerPixel   = 4;
constexpr uint32 g_bytesPerChannel = 8;
constexpr uint32 g_bitsPerPixel    = 32;
constexpr int32	 g_defaultViewportWidth = 640;
constexpr int32	 g_defaultViewportHeight = 480;
constexpr int32	 g_minWindowWidth = 320;
constexpr int32	 g_minWindowHeight = 240;
constexpr int32	 g_maxWindowWidth = 2560;
constexpr int32	 g_maxWindowHeight = 1440;
constexpr int32	 g_maxWindowBufferSize = (g_maxWindowWidth + 1) * (g_maxWindowHeight + 1) * g_bytesPerPixel;
/**
 * @brief Application-specific memory management functions.
 */
namespace ApplicationMemory
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
		memory = nullptr;
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

	template <typename T = void>
	static T* malloc(const size_t size)
	{
		auto tmp = std::malloc(size);
		return (T*)tmp;
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
		free(memory);
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
		std::memset(memory, (int32)value, size);
	}
};
