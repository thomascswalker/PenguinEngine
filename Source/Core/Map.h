#pragma once

#include <map>
#include <type_traits>

#include "Array.h"
#include "Iterator.h"

/** Simple structure for maintaining a relationship between two items. */
template <typename TypeA, typename TypeB>
struct Pair
{
	TypeA a;
	TypeB b;

	Pair() = default;

	Pair(const TypeA& inA, const TypeB& inB)
		: a(inA)
		, b(inB) {}

	Pair(const Pair& other)
	{
		a = other.a;
		b = other.b;
	}

	Pair(Pair&& other) noexcept
	{
		*this = other;
	}

	~Pair() = default;

	Pair& operator=(const Pair& other)
	{
		if (this != &other)
		{
			a = other.a;
			b = other.b;
		}

		return *this;
	}

	Pair& operator=(Pair&& other) noexcept
	{
		if (this != &other)
		{
			a = std::move(other.a);
			b = std::move(other.b);
		}

		return *this;
	}

	friend bool operator==(const Pair& left, const Pair& right)
	{
		return left.a == right.a
			&& left.b == right.b;
	}

	friend bool operator!=(const Pair& left, const Pair& right)
	{
		return !(left == right);
	}

	/** Conversion to STL type. */
	explicit operator std::pair<TypeA, TypeB>() const
	{
		return std::pair(a, b);
	}
};

/**
 * @brief Ordered map of {KeyType: ValueType} items.
 */
template <typename KeyType, typename ValueType>
class Map
{
	// ReSharper disable once CppInconsistentNaming
	using ItemType = Pair<KeyType, ValueType>;
	// ReSharper disable once CppInconsistentNaming
	using IterType = Iterator<Pair<KeyType, ValueType>>;

	Array<ItemType> m_items;

public:
	/**
	 * @brief Default constructor.
	 */
	Map() = default;

	/**
	 * @brief Default destructor.
	 */
	~Map() = default;

	Map(const Map& other)
		: m_items(other.m_items) {}

	Map(Map&& other) noexcept
		: m_items(other.m_items) {}

	/**
	 * @brief Returns the item for the key if it is found. Returns nullptr otherwise.
	 * @param key The key to retrieve the item for.
	 * @return The Pair<KeyType, ValueType> associated with the key.
	 */
	ItemType* get(const KeyType& key)
	{
		for (ItemType& item : m_items)
		{
			if (item.a == key)
			{
				return &item;
			}
		}
		return nullptr;
	}

	/**
	 * @brief Sets the value of the specified key. If the key is found, replaces
	 * the value currently present. If the key is not found, appends a new item to
	 * the end of the item array.
	 * @param key The key to set the value for.
	 * @param value The value to set.
	 */
	void set(const KeyType& key, const ValueType& value)
	{
		ItemType* item = get(key);
		if (item != nullptr)
		{
			item->b = value;
		}
		else
		{
			m_items.append(ItemType(key, value));
		}
	}

	/** Returns true if this map contains the specified key, false otherwise. */
	bool contains(const KeyType& key)
	{
		return get(key) != nullptr;
	}

	/** Removes all items from this map. */
	void clear() const
	{
		m_items.clear();
	}

	/** Returns true if this map contains no items, false otherwise. */
	[[nodiscard]] bool isEmpty() const
	{
		return m_items.isEmpty();
	}

	/** Returns an array of this map's items. */
	[[nodiscard]] Array<ItemType> items() const
	{
		return m_items;
	}

	/** Returns an array of this map's keys. */
	[[nodiscard]] Array<KeyType> keys() const
	{
		Array<KeyType> out;
		for (const auto& [k, v] : m_items)
		{
			out.append(k);
		}
		return out;
	}

	/** Returns an array of this map's values. */
	[[nodiscard]] Array<ValueType> values() const
	{
		Array<ValueType> out;
		for (const auto& [k, v] : m_items)
		{
			out.append(v);
		}
		return out;
	}

	/** The number of items in this map. */
	int32 size()
	{
		return m_items.size();
	}

	/** Returns a pointer to the first element in this map. */
	IterType begin()
	{
		return IterType(m_items.begin());
	}

	/** Returns a pointer to the last element in this map. */
	IterType end()
	{
		return IterType(m_items.end());
	}

	Map& operator=(const Map& other)
	{
		if (this == &other)
		{
			return *this;
		}
		m_items = other.m_items;
		return *this;
	}

	Map& operator=(Map&& other) noexcept
	{
		if (this == &other)
		{
			return *this;
		}
		m_items = std::move(other.m_items);
		return *this;
	}

	// Find or create value by key using operator []
	ValueType& operator[](const KeyType& key)
	{
		ItemType* result = get(key);
		if (result == nullptr)
		{
			set(key, ValueType());
			result = get(key);
		}
		return result->b; // Return reference to the value
	}

	ValueType& operator*() const noexcept
	{
		return const_cast<ValueType>(begin());
	}

	ValueType* operator->() const noexcept
	{
		return begin();
	}

	/** Conversion to STL type. */
	explicit operator std::map<KeyType, ValueType>() const
	{
		std::map<KeyType, ValueType> out;
		for (int32 index = 0; index < size(); index++)
		{
			auto item = m_items[index];
			out[item.a] = item.b;
		}
		return out;
	}
};