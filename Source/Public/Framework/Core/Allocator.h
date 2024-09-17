#pragma once

#include <iterator>
#include <type_traits>
#include <utility>
#include <vector>

template <typename T>
T nextMultiple(const T n)
{
	return (n + 2 - 1) & -2;
}

template <typename T>
class IAllocator
{
public:
	IAllocator() = default;
	virtual ~IAllocator() = default;

	virtual T* allocate(size_t n) = 0;
	virtual void deallocate(T* ptr, size_t n) = 0;
	virtual void construct(T* ptr, const T& value) = 0;
	virtual void destroy(T* ptr) = 0;
};

/**
 * @brief Simple default m_allocator.
 */
template <typename T>
class DefaultAllocator : public IAllocator<T>
{
public:
	/**
	 * @brief Allocates new memory the m_size of (T * n).
	 * @param n The byte m_size to allocate.
	 * @return T* Type T pointer to the allocated memory.
	 */
	T* allocate(size_t n) override
	{
		return static_cast<T*>(std::malloc(n * sizeof(T)));
	}

	/**
	 * @brief Deletes the specified pointer.
	 * @param ptr The pointer to delete.
	 * @param n The byte m_size to deallocate. TODO: Implement this.
	 */
	void deallocate(T* ptr, size_t n) override
	{
		delete(ptr);
	}

	/**
	 * @brief 	Construct a new object (value) in the position of the pointer (ptr).
	 *
	 * Placement new: This does NOT allocate new memory, it just constructs an object in an already-allocated memory space.
	 * @param ptr The memory space to construct the object in.
	 * @param value The object to construct.
	 */

	void construct(T* ptr, const T& value) override
	{
		new(ptr)T(value);
	}

	template <typename... Args>
	void construct(T* ptr, Args&&... args)
	{
		new(ptr)T(std::forward<Args>(args)...);
	}

	/**
	 * @brief Calls the destructor on the specified memory.
	 * @param ptr The memory to destroy.
	 */
	void destroy(T* ptr) override
	{
		if (ptr != nullptr && (!std::is_trivially_default_constructible_v<T>))
		{
			ptr->~T();
		}
	}
};
