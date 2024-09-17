// ReSharper disable CppInconsistentNaming
#pragma once

// https://www.internalpointers.com/post/writing-custom-iterators-modern-cpp
template <typename T>
struct Iterator
{
	using IteratorCategory = std::forward_iterator_tag;
	using DifferenceType = std::ptrdiff_t;
	using ValueType = T;
	using PointerType = T*;
	using ReferenceType = T&;

private:
	PointerType m_ptr;

public:
	explicit Iterator(const PointerType ptr)
		: m_ptr(ptr) {}

	ReferenceType operator*() const
	{
		return *m_ptr;
	}

	PointerType operator->() const
	{
		return m_ptr;
	}

	// Prefix increment
	Iterator& operator++()
	{
		++m_ptr;
		return *this;
	}

	// Postfix increment
	Iterator operator++(int)
	{
		Iterator tmp = *this;
		++(*this);
		return tmp;
	}

	friend bool operator==(const Iterator& a, const Iterator& b)
	{
		return a.m_ptr == b.m_ptr;
	}

	friend bool operator!=(const Iterator& a, const Iterator& b)
	{
		return a.m_ptr != b.m_ptr;
	}
};
