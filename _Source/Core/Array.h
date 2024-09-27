#pragma once

#include <stdexcept>
#include <type_traits>
#include <utility>

#include "Allocator.h"
#include "Iterator.h"
#include "Types.h"

constexpr int32 g_notFoundIndex = -1;

template <typename T, typename Allocator = DefaultAllocator<T>>
class Array
{
	// ReSharper disable once CppInconsistentNaming
	using IterType = Iterator<T>;

	/* The m_allocator which manages this array's memory. */
	Allocator m_allocator;
	/* The type T pointer to this array's memory. */
	T* m_data = nullptr;
	/* The m_size of objects within this array. */
	int32 m_size = 0;
	/* The total m_capacity of this array. */
	int32 m_capacity = 0;

	/**
	 * @brief Resizes this array's m_capacity by doubling it. All existing m_data is copied to a
	 * temporary buffer and the copied back to this array's m_data buffer.
	 */
	void resize(const int32 newSize)
	{
		int32 newCapacity = newSize == 0 ? 1 : nextMultiple(newSize);
		T*    newData = m_allocator.allocate(newCapacity);
		for (int32 index = 0; index < m_size; index++)
		{
			m_allocator.construct(&newData[index], std::move(m_data[index]));
			m_allocator.destroy(&m_data[index]);
		}

		m_allocator.deallocate(m_data, m_capacity);

		m_data = newData;
		m_capacity = newCapacity;
	}

	/**
	 * @brief Shrinks this array's m_capacity if the current m_size has gone below 1/2 of the current
	 * m_capacity. All existing m_data is copied to a temporary buffer and the copied back to this
	 * array's m_data buffer.
	 */
	void shrink()
	{
		if (m_size < m_capacity / 2)
		{
			int32 newCapacity = m_size == 0 ? 1 : nextMultiple(m_size); // New m_capacity should be at least the m_size
			T*    newData = m_allocator.allocate(newCapacity);

			// Move old m_data to the new block
			for (int32 index = 0; index < m_size; index++)
			{
				m_allocator.construct(&newData[index], std::move(m_data[index]));
				m_allocator.destroy(&m_data[index]);
			}

			m_allocator.deallocate(m_data, m_capacity); // Free old memory
			m_data = newData;
			m_capacity = newCapacity;
		}
	}

public:
	/**
	 * @brief Default constructor.
	 */
	Array()
		: m_data(nullptr) {}

	/**
	 * @brief Initializer list constructor.
	 */
	Array(const std::initializer_list<T>& values)
	{
		m_size = values.size();
		int32 newCapacity = m_size == 0 ? 1 : ((m_size + 1) / 2) * 2;
		reserve(newCapacity);

		for (int32 index = 0; index < values.size(); index++)
		{
			T item = *(values.begin() + index);
			m_data[index] = item;
		}
	}

	/**
	 * @brief Copy constructor (deep copy).
	 */
	Array(const Array& other)
		: m_allocator(other.m_allocator)
		, m_data(nullptr)
		, m_size(other.m_size)
		, m_capacity(other.m_capacity)
	{
		m_data = m_allocator.allocate(m_capacity);
		for (int32 index = 0; index < m_size; index++)
		{
			m_allocator.construct(&m_data[index], std::move(other.m_data[index]));
		}
	}

	/**
	 * @brief Move constructor (shallow copy, transfer ownership).
	 */
	Array(Array&& other) noexcept
		: m_allocator(other.m_allocator)
		, m_data(other.m_data)
		, m_size(other.m_size)
		, m_capacity(other.m_capacity)
	{
		other.m_data = nullptr; // Leave the source in an empty state
		other.m_capacity = 0;
		other.m_size = 0;
	}

	/**
	 * Copy assignment operator (deep copy).
	 * TODO: Fix nested arrays.
	 */
	Array& operator=(const Array& other)
	{
		if (this == &other)
		{
			return *this; // Handle self-assignment
		}

		m_size = other.m_size;
		m_capacity = other.m_capacity;
		std::free(m_data);
		m_data = nullptr;
		m_data = m_allocator.allocate(m_capacity);
		for (int32 index = 0; index < m_size; index++)
		{
			m_allocator.construct(&m_data[index], std::move(other.m_data[index]));
		}

		return *this;
	}

	/**
	 * Move assignment operator (shallow copy, transfer ownership).
	 */
	Array& operator=(Array&& other) noexcept
	{
		if (this == &other)
		{
			return *this; // Handle self-assignment
		}

		clear();                                    // Destroy current elements
		m_allocator.deallocate(m_data, m_capacity); // Free old memory

		// Transfer ownership of m_data
		m_data = other.m_data;
		m_capacity = other.m_capacity;
		m_size = other.m_size;
		m_allocator = other.m_allocator;

		// Leave the source in an empty state
		other.m_data = nullptr;
		other.m_capacity = 0;
		other.m_size = 0;

		return *this;
	}

	/**
	 * Default destructor.
	 */
	~Array()
	{
		clear();
		m_allocator.deallocate(m_data, m_capacity);
	}

	/**
	 * @brief Reserves the specified m_capacity in this array.
	 * @param newCapacity The number of bytes to reserve.
	 */
	void reserve(int32 newCapacity)
	{
		if (newCapacity <= m_capacity)
		{
			return;
		}

		T* newData = m_allocator.allocate(newCapacity);
		if (m_data != nullptr)
		{
			for (int32 index = 0; index < m_size; index++)
			{
				m_allocator.construct(&newData[index], std::move(m_data[index]));
				m_allocator.destroy(&m_data[index]);
			}

			m_allocator.deallocate(m_data, newCapacity);
		}

		m_data = newData;
		m_capacity = newCapacity;
	}

	/**
	 * @brief Grows the m_capacity of this array by m_size n.
	 * @param n The number of bytes to grow the m_capacity by.
	 */
	void grow(const int32 n)
	{
		int32 newCapacity = m_capacity + n;
		reserve(newCapacity);
	}

	/**
	 * @brief Appends the specified item to the end of the array. Resizes the array if necessary.
	 * @param item The item to append.
	 */
	void append(const T& item)
	{
		if (m_size == m_capacity)
		{
			resize(m_size + 1); // If we're out of space, resize
		}
		m_allocator.construct(&m_data[m_size], item); // Construct the new item
		m_size++;
	}

	/**
	 * @brief Extend this array by appending all of the items from another array.
	 * @param other The other array.
	 */
	void extend(const Array<T>& other)
	{
		int32 oldSize = m_size;
		int32 newSize = oldSize + other.size();
		if (newSize >= m_capacity)
		{
			resize(newSize); // If we're out of space, resize
		}
		for (int32 index = 0; index < other.size(); index++)
		{
			m_allocator.construct(&m_data[m_size + index], std::move(other[index]));
		}
		m_size = newSize;
	}

	/**
	 * @brief Removes the item at the specified position in the array.
	 * @param index The index of the item to remove.
	 */
	void remove(int32 index)
	{
		m_allocator.destroy(&m_data[index]);

		for (/* index is an argument */; index < m_size - 1; index++)
		{
			m_allocator.construct(&m_data[index], std::move(m_data[index + 1]));
			m_allocator.destroy(&m_data[index + 1]);
		}

		m_size--;
		shrink();
	}

	/**
	 * @brief Clears this array of all of its content and deallocates the memory used.
	 */
	void clear()
	{
		if (m_data != nullptr)
		{
			for (int32 index = 0; index < m_size; index++)
			{
				m_allocator.destroy(&m_data[index]);
			}
		}

		m_data = nullptr;
		m_size = 0;
		m_capacity = 0;
	}

	/**
	 * @brief Returns the index of the specified item. Returns -1 if not found.
	 * @param item The item to find.
	 * @return The index of the item.
	 */
	int32 index(const T& item)
	{
		for (int32 index = 0; index < m_size; index++)
		{
			if (item == m_data[index])
			{
				return index;
			}
		}
		return g_notFoundIndex;
	}

	/**
	 * @brief Returns true if the specified item is found, false otherwise.
	 * @param item The item to find.
	 */
	bool contains(const T& item)
	{
		return index(item) != g_notFoundIndex;
	}

	/** Returns the m_size of the array. */
	[[nodiscard]] int32 size() const
	{
		return m_size;
	}

	/** Returns true if the array is empty, false otherwise. */
	[[nodiscard]] bool isEmpty() const
	{
		return m_size == 0;
	}

	/** Returns a pointer to the first element in this array. */
	[[nodiscard]] IterType begin() const
	{
		return IterType(m_data);
	}

	/** Returns a pointer to the last element in this array. */
	[[nodiscard]] IterType end() const
	{
		return IterType(m_data + m_size);
	}

	T& operator[](int32 index)
	{
		if (index >= m_size)
		{
			throw std::out_of_range("Index out of bounds");
		}

		// If index is negative, loop around to the end of the array and negate from there
		if (index < 0)
		{
			index = m_size + index;
		}
		return m_data[index];
	}

	const T& operator[](int32 index) const
	{
		if (index >= m_size)
		{
			throw std::out_of_range("Index out of bounds");
		}
		// If index is negative, loop around to the end of the array and negate from there
		if (index < 0)
		{
			index = m_size + index;
		}
		return m_data[index];
	}

	/** Conversion to STL type. */
	explicit operator std::vector<T>() const
	{
		std::vector<T> out;
		for (int32 index = 0; index < m_size; index++)
		{
			out.emplace_back(m_data[index]);
		}
		return out;
	}
};