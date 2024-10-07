#pragma once

#include <assert.h>

#include "Object.h"

constexpr uint32 g_maxObjectCount = 10000;

// https://austinmorlan.com/posts/entity_component_system/
class ObjectManager
{
	std::array<Object*, g_maxObjectCount> m_objects{};

	std::queue<ObjectId> m_availableIds{};
	uint32 m_objectCount = 0;

public:
	ObjectManager()
	{
		// Initialize all available object ID slots
		for (ObjectId id = 0; id < g_maxObjectCount; id++)
		{
			m_availableIds.push(id);
		}
	}

	~ObjectManager()
	{
		destroyAll();
	}

	template <typename T>
	T* createObject()
	{
		static_assert(std::is_base_of_v<Object, T>, "Class of type T is not derived from Object.");

		// Get the next available ID
		ObjectId id = m_availableIds.front();

		// Remove this ID from the available ID list 
		m_availableIds.pop();

		// Construct in place a new object of type T
		m_objects[id]     = new T();
		Object* newObject = m_objects[id];
		newObject->setObjectId(id);

		// Bump object count
		m_objectCount++;

		// Return the new object cast to type T
		return dynamic_cast<T*>(newObject);
	}

	template <typename T>
	void destroyObject(T* object)
	{
		static_assert(std::is_base_of_v<Object, T>, "Class of type T is not derived from Object.");

		auto id       = object->getObjectId();
		m_objects[id] = nullptr;
		object->~T();
		m_availableIds.push(id);
	}

	template <typename T>
	T* getObject(const ObjectId objectId)
	{
		static_assert(std::is_base_of_v<Object, T>, "Class of type T is not derived from Object.");

		return dynamic_cast<T*>(m_objects[objectId]);
	}

	void destroyAll()
	{
		for (Object* obj : m_objects)
		{
			if (obj != nullptr)
			{
				destroyObject(obj);
			}
		}
	}

	[[nodiscard]] std::vector<IRenderable*> getRenderables() const
	{
		std::vector<IRenderable*> out;

		for (int32 i = 0; i < m_objectCount; i++)
		{
			Object* obj = m_objects[i];
			if (obj == nullptr)
			{
				continue;
			}
			if (obj->hasSignature(ESignature::Renderable))
			{
				out.emplace_back(dynamic_cast<IRenderable*>(obj));
			}
		}

		return out;
	}

	[[nodiscard]] std::vector<ITickable*> getTickables() const
	{
		std::vector<ITickable*> out;

		for (int32 i = 0; i < m_objectCount; i++)
		{
			Object* obj = m_objects[i];
			if (obj == nullptr)
			{
				continue;
			}
			if (obj->hasSignature(ESignature::Tickable))
			{
				out.emplace_back(dynamic_cast<ITickable*>(obj));
			}
		}

		return out;
	}
};

inline ObjectManager g_objectManager;
