#pragma once

#include <stdexcept>
#include "Core/Types.h"
#include "Core/Logging.h"
#include "Core/Iterator.h"

template <typename T> class LinkedList
{
public:
	class Node
	{
	protected:
		Node* next = nullptr;
		Node* prev = nullptr;
		T	  value{};

	public:
		friend class LinkedList;

		Node() {}
		Node(T newValue) : value(newValue), next(nullptr), prev(nullptr) {}

		const T& getValue() const { return value; }
		T&		 getValue() { return value; }

		const Node* getNext() const { return next; }
		Node*		getNext() { return next; }
		const Node* getPrev() const { return prev; }
		Node*		getPrev() { return prev; }
	};

private:
	Node* m_head = nullptr;
	Node* m_tail = nullptr;
	int32 m_size = 0;

public:
	using ListIterator = Iterator<Node>;
	using ConstListIterator = const Iterator<Node>;

	LinkedList() {}
	~LinkedList() { clear(); }

	int32 size() const { return m_size; }
	bool  isEmpty() const { return m_size == 0; }
	Node* getFront() const { return m_head; }
	Node* getBack() const { return m_tail; }

	/* Adds the specified new node at the start of the list. */
	void addFront(Node* newNode)
	{
		if (m_head != nullptr)
		{
			newNode->next = m_head;
			m_head->prev = newNode;
			m_head = newNode;
			m_tail->next = m_head;
		}
		else
		{
			m_head = m_tail = newNode;
		}

		m_size++;
	}

	void addFront(const T& value) { addFront(new Node(value)); }

	void addBack(Node* newNode)
	{
		if (m_tail != nullptr)
		{
			m_tail->next = newNode;
			newNode->prev = m_tail;
			newNode->next = m_head;
			m_tail = newNode;
			m_head->prev = m_tail;
		}
		else
		{
			m_head = m_tail = newNode;
		}

		m_size++;
	}

	void addBack(const T& value) { addBack(new Node(value)); }

	void clear()
	{
		Node* node;

		while (m_head != nullptr && m_size > 0)
		{
			node = m_head->next;
			delete m_head;
			m_head = node;
			m_size--;
		}

		m_head = nullptr;
		m_tail = nullptr;
	}

	Node* at(int32 index)
	{
		Node* current = m_head;
		for (int32 i = 0; i < index; i++)
		{
			current = current->next;
		}
		return current;
	}

	void remove(Node* node)
	{
		if (node == nullptr)
		{
			return;
		}

		if (m_size == 1)
		{
			clear();
			return;
		}

		if (node == m_head)
		{
			m_head = m_head->next;
			m_head->prev = m_tail;
			m_tail->next = m_head;
		}
		else if (node == m_tail)
		{
			m_tail = m_tail->prev;
			m_tail->next = m_head;
			m_head->prev = m_tail;
		}
		else
		{
			node->next->prev = node->prev;
			node->prev->next = node->next;
		}

		delete node;
		m_size--;
	}

	Node* find(const T& value)
	{
		Node* node = m_head;
		while (node != nullptr)
		{
			if (node->getValue() == value)
			{
				break;
			}
			node = node->next;
		}
		return node;
	}

	bool contains(const T& value) { return find(value) != nullptr; }

	friend ListIterator begin(LinkedList& list) { return ListIterator(list.getFront()); }
	friend ConstListIterator begin(const LinkedList& list) { return ConstListIterator(list.getFront()); }
	friend ListIterator end(LinkedList& list) { return ListIterator(nullptr); }
	friend ConstListIterator end(const LinkedList& list) { return ConstListIterator(nullptr); }
};