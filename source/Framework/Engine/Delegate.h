// ReSharper disable CppInconsistentNaming
#pragma once

#include <vector>
#include <memory>
#include <tuple>

#ifndef DELEGATE_ASSERT
#include <assert.h>
#define DELEGATE_ASSERT(Expr, ...) assert(Expr)
#endif

#define DELEGATE_STATIC_ASSERT(Expr, Msg) static_assert(Expr, Msg)

//The allocation Size of delegate compressedData.
//Delegates larger than this will be heap allocated.
#ifndef DELEGATE_INLINE_ALLOCATION_SIZE
#define DELEGATE_INLINE_ALLOCATION_SIZE 32
#endif

#define DECLARE_DELEGATE(name, ...) \
using name = Delegate<void, __VA_ARGS__>

#define DECLARE_DELEGATE_RET(name, retValue, ...) \
using name = Delegate<retValue, __VA_ARGS__>

#define DECLARE_MULTICAST_DELEGATE(name, ...) \
using name = MulticastDelegate<__VA_ARGS__>; \
using name ## Delegate = MulticastDelegate<__VA_ARGS__>::DelegateT

#define DECLARE_EVENT(name, ownerType, ...) \
class name : public MulticastDelegate<__VA_ARGS__> \
{ \
private: \
	friend class ownerType; \
	using MulticastDelegate::broadcast; \
	using MulticastDelegate::removeAll; \
	using MulticastDelegate::remove; \
};

namespace _DelegatesInternal
{
	template <bool IsConst, typename Object, typename RetVal, typename... Args>
	struct MemberFunction;

	template <typename Object, typename RetVal, typename... Args>
	struct MemberFunction<true, Object, RetVal, Args...>
	{
		using Type = RetVal(Object::*)(Args...) const;
	};

	template <typename Object, typename RetVal, typename... Args>
	struct MemberFunction<false, Object, RetVal, Args...>
	{
		using Type = RetVal(Object::*)(Args...);
	};

	static void* (*alloc)(size_t size) = [](size_t size)
	{
		return malloc(size);
	};
	static void (*free)(void* ptr) = [](void* ptr)
	{
		free(ptr);
	};

	template <typename T>
	void DelegateDeleteFunc(T* ptr)
	{
		ptr->~T();
		DelegateFreeFunc(ptr);
	}
}

namespace Delegates
{
	using AllocateCallback = void* (*)(size_t size);
	using FreeCallback = void(*)(void* pPtr);

	inline void SetAllocationCallbacks(const AllocateCallback allocateCallback, const FreeCallback freeCallback)
	{
		_DelegatesInternal::alloc = allocateCallback;
		_DelegatesInternal::free = freeCallback;
	}
}

class IDelegateBase
{
public:
	IDelegateBase() = default;
	virtual ~IDelegateBase() noexcept = default;

	virtual const void* getOwner() const
	{
		return nullptr;
	}

	virtual void clone(void* pDestination) = 0;
};

//Base type for delegates
template <typename RetVal, typename... Args>
class IDelegate : public IDelegateBase
{
public:
	virtual RetVal execute(Args&&... args) = 0;
};

template <typename RetVal, typename... Args2>
class StaticDelegate;

template <typename RetVal, typename... Args, typename... Args2>
class StaticDelegate<RetVal(Args...), Args2...> : public IDelegate<RetVal, Args...>
{
public:
	using DelegateFunction = RetVal(*)(Args..., Args2...);

	StaticDelegate(DelegateFunction Func, Args2&&... payload)
		: m_Function(Func), m_Payload(std::forward<Args2>(payload)...) {}

	StaticDelegate(DelegateFunction function, const std::tuple<Args2...>& payload)
		: m_Function(function), m_Payload(payload) {}

	RetVal execute(Args&&... args) override
	{
		return executeInternal(std::forward<Args>(args)..., std::index_sequence_for<Args2...>());
	}

	void clone(void* pDestination) override
	{
		new(pDestination) StaticDelegate(m_Function, m_Payload);
	}

private:
	template <std::size_t... Is>
	RetVal executeInternal(Args&&... args, std::index_sequence<Is...>)
	{
		return m_Function(std::forward<Args>(args)..., std::get<Is>(m_Payload)...);
	}

	DelegateFunction m_Function;
	std::tuple<Args2...> m_Payload;
};

template <bool IsConst, typename T, typename RetVal, typename... Args2>
class RawDelegate;

template <bool IsConst, typename T, typename RetVal, typename... Args, typename... Args2>
class RawDelegate<IsConst, T, RetVal(Args...), Args2...> : public IDelegate<RetVal, Args...>
{
public:
	using DelegateFunction = typename _DelegatesInternal::MemberFunction<IsConst, T, RetVal, Args..., Args2...>::Type;

	RawDelegate(T* pObject, DelegateFunction function, Args2&&... payload)
		: m_pObject(pObject), m_Function(function), m_Payload(std::forward<Args2>(payload)...) {}

	RawDelegate(T* pObject, DelegateFunction function, const std::tuple<Args2...>& payload)
		: m_pObject(pObject), m_Function(function), m_Payload(payload) {}

	RetVal execute(Args&&... args) override
	{
		return executeInternal(std::forward<Args>(args)..., std::index_sequence_for<Args2...>());
	}

	const void* getOwner() const override
	{
		return m_pObject;
	}

	void clone(void* pDestination) override
	{
		new(pDestination) RawDelegate(m_pObject, m_Function, m_Payload);
	}

private:
	template <std::size_t... Is>
	RetVal executeInternal(Args&&... args, std::index_sequence<Is...>)
	{
		return (m_pObject->*m_Function)(std::forward<Args>(args)..., std::get<Is>(m_Payload)...);
	}

	T* m_pObject;
	DelegateFunction m_Function;
	std::tuple<Args2...> m_Payload;
};

template <typename TLambda, typename RetVal, typename... Args>
class LambdaDelegate;

template <typename TLambda, typename RetVal, typename... Args, typename... Args2>
class LambdaDelegate<TLambda, RetVal(Args...), Args2...> : public IDelegate<RetVal, Args...>
{
public:
	explicit LambdaDelegate(TLambda&& lambda, Args2&&... payload)
		: m_Lambda(std::forward<TLambda>(lambda)),
		  m_Payload(std::forward<Args2>(payload)...) {}

	explicit LambdaDelegate(const TLambda& lambda, const std::tuple<Args2...>& payload)
		: m_Lambda(lambda),
		  m_Payload(payload) {}

	RetVal execute(Args&&... args) override
	{
		return executeInternal(std::forward<Args>(args)..., std::index_sequence_for<Args2...>());
	}

	void clone(void* pDestination) override
	{
		new(pDestination) LambdaDelegate(m_Lambda, m_Payload);
	}

private:
	template <std::size_t... Is>
	RetVal executeInternal(Args&&... args, std::index_sequence<Is...>)
	{
		return (RetVal)((m_Lambda)(std::forward<Args>(args)..., std::get<Is>(m_Payload)...));
	}

	TLambda m_Lambda;
	std::tuple<Args2...> m_Payload;
};

template <bool IsConst, typename T, typename RetVal, typename... Args>
class SPDelegate;

template <bool IsConst, typename RetVal, typename T, typename... Args, typename... Args2>
class SPDelegate<IsConst, T, RetVal(Args...), Args2...> : public IDelegate<RetVal, Args...>
{
public:
	using DelegateFunction = typename _DelegatesInternal::MemberFunction<IsConst, T, RetVal, Args..., Args2...>::Type;

	SPDelegate(std::shared_ptr<T> pObject, DelegateFunction pFunction, Args2&&... payload)
		: m_pObject(pObject),
		  m_pFunction(pFunction),
		  m_Payload(std::forward<Args2>(payload)...) {}

	SPDelegate(std::weak_ptr<T> pObject, DelegateFunction pFunction, const std::tuple<Args2...>& payload)
		: m_pObject(pObject),
		  m_pFunction(pFunction),
		  m_Payload(payload) {}

	RetVal execute(Args&&... args) override
	{
		return executeInternal(std::forward<Args>(args)..., std::index_sequence_for<Args2...>());
	}

	const void* getOwner() const override
	{
		return m_pObject.expired() ? nullptr : m_pObject.lock().get();
	}

	void clone(void* pDestination) override
	{
		new(pDestination) SPDelegate(m_pObject, m_pFunction, m_Payload);
	}

private:
	template <std::size_t... Is>
	RetVal executeInternal(Args&&... args, std::index_sequence<Is...>)
	{
		if (m_pObject.expired())
		{
			return RetVal();
		}
		else
		{
			std::shared_ptr<T> pPinned = m_pObject.lock();
			return (pPinned.get()->*m_pFunction)(std::forward<Args>(args)..., std::get<Is>(m_Payload)...);
		}
	}

	std::weak_ptr<T> m_pObject;
	DelegateFunction m_pFunction;
	std::tuple<Args2...> m_Payload;
};

//a handle to a delegate used for a multicast delegate
//Static ID so that every handle is unique
class DelegateHandle
{
public:
	constexpr DelegateHandle() noexcept
		: m_Id(INVALID_ID) {}

	explicit DelegateHandle(bool /*generateId*/) noexcept
		: m_Id(getNewId()) {}

	~DelegateHandle() noexcept = default;
	DelegateHandle(const DelegateHandle& other) = default;
	DelegateHandle& operator=(const DelegateHandle& other) = default;

	DelegateHandle(DelegateHandle&& other) noexcept
		: m_Id(other.m_Id)
	{
		other.reset();
	}

	DelegateHandle& operator=(DelegateHandle&& other) noexcept
	{
		m_Id = other.m_Id;
		other.reset();
		return *this;
	}

	operator bool() const noexcept
	{
		return isValid();
	}

	bool operator==(const DelegateHandle& other) const noexcept
	{
		return m_Id == other.m_Id;
	}

	bool operator<(const DelegateHandle& other) const noexcept
	{
		return m_Id < other.m_Id;
	}

	bool isValid() const noexcept
	{
		return m_Id != INVALID_ID;
	}

	void reset() noexcept
	{
		m_Id = INVALID_ID;
	}

	constexpr static const unsigned int INVALID_ID = (unsigned int)~0;

private:
	unsigned int m_Id;
	inline static unsigned int CURRENT_ID = 0;

	static int getNewId()
	{
		unsigned int output = CURRENT_ID++;
		if (CURRENT_ID == INVALID_ID)
		{
			CURRENT_ID = 0;
		}
		return output;
	}
};

template <size_t MaxStackSize>
class InlineAllocator
{
public:
	//Constructor
	constexpr InlineAllocator() noexcept
		: m_size(0)
	{
		DELEGATE_STATIC_ASSERT(MaxStackSize > sizeof(void*),
		                       "MaxStackSize is smaller or equal to the Size of a pointer. This will make the use of an InlineAllocator pointless. Please increase the MaxStackSize.")
			;
	}

	//Destructor
	~InlineAllocator() noexcept
	{
		free();
	}

	//Copy constructor
	InlineAllocator(const InlineAllocator& other)
		: m_size(0)
	{
		if (other.hasAllocation())
		{
			memcpy(allocate(other.m_size), other.getAllocation(), other.m_size);
		}
		m_size = other.m_size;
	}

	//Copy assignment operator
	InlineAllocator& operator=(const InlineAllocator& other)
	{
		if (other.hasAllocation())
		{
			memcpy(allocate(other.m_size), other.getAllocation(), other.m_size);
		}
		m_size = other.m_size;
		return *this;
	}

	//Move constructor
	InlineAllocator(InlineAllocator&& other) noexcept
		: m_size(other.m_size)
	{
		other.m_size = 0;
		if (m_size > MaxStackSize)
		{
			std::swap(pPtr, other.pPtr);
		}
		else
		{
			memcpy(buffer, other.buffer, m_size);
		}
	}

	//Move assignment operator
	InlineAllocator& operator=(InlineAllocator&& other) noexcept
	{
		free();
		m_size = other.m_size;
		other.m_size = 0;
		if (m_size > MaxStackSize)
		{
			std::swap(pPtr, other.pPtr);
		}
		else
		{
			memcpy(buffer, other.buffer, m_size);
		}
		return *this;
	}

	//allocate memory of given Size
	//If the Size is over the predefined threshold, it will be allocated on the heap
	void* allocate(const size_t size)
	{
		if (m_size != size)
		{
			free();
			m_size = size;
			if (size > MaxStackSize)
			{
				pPtr = _DelegatesInternal::alloc(size);
				return pPtr;
			}
		}
		return (void*)buffer;
	}

	//Free the allocated memory
	void free()
	{
		if (m_size > MaxStackSize)
		{
			_DelegatesInternal::free(pPtr);
		}
		m_size = 0;
	}

	//Return the allocated memory either on the stack or on the heap
	void* getAllocation() const
	{
		if (hasAllocation())
		{
			return hasHeapAllocation() ? pPtr : (void*)buffer;
		}
		else
		{
			return nullptr;
		}
	}

	[[nodiscard]] size_t getSize() const
	{
		return m_size;
	}

	[[nodiscard]] bool hasAllocation() const
	{
		return m_size > 0;
	}

	[[nodiscard]] bool hasHeapAllocation() const
	{
		return m_size > MaxStackSize;
	}

private:
	//If the allocation is smaller than the threshold, buffer is used
	//Otherwise Ptr is used together with a separate dynamic allocation
	union
	{
		char buffer[MaxStackSize];
		void* pPtr;
	};

	size_t m_size = 0;
};

class DelegateBase
{
public:
	//Default constructor
	constexpr DelegateBase() noexcept {}

	//Default destructor
	virtual ~DelegateBase() noexcept
	{
		release();
	}

	//Copy contructor
	DelegateBase(const DelegateBase& other)
	{
		if (other.m_allocator.hasAllocation())
		{
			m_allocator.allocate(other.m_allocator.getSize());
			other.getDelegate()->clone(m_allocator.getAllocation());
		}
	}

	//Copy assignment operator
	DelegateBase& operator=(const DelegateBase& other)
	{
		release();
		if (other.m_allocator.hasAllocation())
		{
			m_allocator.allocate(other.m_allocator.getSize());
			other.getDelegate()->clone(m_allocator.getAllocation());
		}
		return *this;
	}

	//Move constructor
	DelegateBase(DelegateBase&& other) noexcept
		: m_allocator(std::move(other.m_allocator)) {}

	//Move assignment operator
	DelegateBase& operator=(DelegateBase&& other) noexcept
	{
		release();
		m_allocator = std::move(other.m_allocator);
		return *this;
	}

	//Gets the owner of the deletage
	//Only valid for SPDelegate and RawDelegate.
	//Otherwise returns nullptr by default
	[[nodiscard]] const void* getOwner() const
	{
		if (m_allocator.hasAllocation())
		{
			return getDelegate()->getOwner();
		}
		return nullptr;
	}

	[[nodiscard]] size_t getSize() const
	{
		return m_allocator.getSize();
	}

	//Clear the bound delegate if it is bound to the given object.
	//Ignored when pObject is a nullptr
	void clearIfBoundTo(void* pObject)
	{
		if (pObject != nullptr && isBoundTo(pObject))
		{
			release();
		}
	}

	//Clear the bound delegate if it exists
	void clear()
	{
		release();
	}

	//If the allocator has a Size, it means it's bound to something
	bool isBound() const
	{
		return m_allocator.hasAllocation();
	}

	bool isBoundTo(void* pObject) const
	{
		if (pObject == nullptr || m_allocator.hasAllocation() == false)
		{
			return false;
		}
		return getDelegate()->getOwner() == pObject;
	}

protected:
	void release()
	{
		if (m_allocator.hasAllocation())
		{
			getDelegate()->~IDelegateBase();
			m_allocator.free();
		}
	}

	IDelegateBase* getDelegate() const
	{
		return static_cast<IDelegateBase*>(m_allocator.getAllocation());
	}

	//Allocator for the delegate itself.
	//Delegate gets allocated when its is smaller or equal than 64 bytes in Size.
	//Can be changed by preference
	InlineAllocator<DELEGATE_INLINE_ALLOCATION_SIZE> m_allocator;
};

//Delegate that can be bound to by just ONE object
template <typename RetVal, typename... Args>
class Delegate : public DelegateBase
{
private:
	template <typename T, typename... Args2>
	using ConstMemberFunction = typename _DelegatesInternal::MemberFunction<true, T, RetVal, Args..., Args2...>::Type;
	template <typename T, typename... Args2>
	using NonConstMemberFunction = typename _DelegatesInternal::MemberFunction<
		false, T, RetVal, Args..., Args2...>::Type;

public:
	using IDelegateT = IDelegate<RetVal, Args...>;

	//Create delegate using member function
	template <typename T, typename... Args2>
	[[nodiscard]] static Delegate createRaw(T* pObj, NonConstMemberFunction<T, Args2...> pFunction, Args2... args)
	{
		Delegate handler;
		handler.bind<RawDelegate<false, T, RetVal(Args...), Args2...>>(pObj, pFunction, std::forward<Args2>(args)...);
		return handler;
	}

	template <typename T, typename... Args2>
	[[nodiscard]] static Delegate createRaw(T* pObj, ConstMemberFunction<T, Args2...> pFunction, Args2... args)
	{
		Delegate handler;
		handler.bind<RawDelegate<true, T, RetVal(Args...), Args2...>>(pObj, pFunction, std::forward<Args2>(args)...);
		return handler;
	}

	//Create delegate using global/static function
	template <typename... Args2>
	[[nodiscard]] static Delegate createStatic(RetVal (*pFunction)(Args..., Args2...), Args2... args)
	{
		Delegate handler;
		handler.bind<StaticDelegate<RetVal(Args...), Args2...>>(pFunction, std::forward<Args2>(args)...);
		return handler;
	}

	//Create delegate using std::shared_ptr
	template <typename T, typename... Args2>
	[[nodiscard]] static Delegate createSp(const std::shared_ptr<T>& pObject,
	                                       NonConstMemberFunction<T, Args2...> pFunction, Args2... args)
	{
		Delegate handler;
		handler.bind<SPDelegate<false, T, RetVal(Args...), Args2...>>(pObject, pFunction, std::forward<Args2>(args)...);
		return handler;
	}

	template <typename T, typename... Args2>
	[[nodiscard]] static Delegate createSp(const std::shared_ptr<T>& pObject,
	                                       ConstMemberFunction<T, Args2...> pFunction,
	                                       Args2... args)
	{
		Delegate handler;
		handler.bind<SPDelegate<true, T, RetVal(Args...), Args2...>>(pObject, pFunction, std::forward<Args2>(args)...);
		return handler;
	}

	//Create delegate using a lambda
	template <typename TLambda, typename... Args2>
	[[nodiscard]] static Delegate createLambda(TLambda&& lambda, Args2... args)
	{
		Delegate handler;
		handler.bind<LambdaDelegate<TLambda, RetVal(Args...), Args2...>>(
			std::forward<TLambda>(lambda), std::forward<Args2>(args)...);
		return handler;
	}

	//bind a member function
	template <typename T, typename... Args2>
	void bindRaw(T* pObject, NonConstMemberFunction<T, Args2...> pFunction, Args2&&... args)
	{
		DELEGATE_STATIC_ASSERT(!std::is_const<T>::value, "Cannot bind a non-const function on a const object");
		*this = createRaw<T, Args2...>(pObject, pFunction, std::forward<Args2>(args)...);
	}

	template <typename T, typename... Args2>
	void bindRaw(T* pObject, ConstMemberFunction<T, Args2...> pFunction, Args2&&... args)
	{
		*this = createRaw<T, Args2...>(pObject, pFunction, std::forward<Args2>(args)...);
	}

	//bind a static/global function
	template <typename... Args2>
	void bindStatic(RetVal (*pFunction)(Args..., Args2...), Args2&&... args)
	{
		*this = CreateStatic<Args2...>(pFunction, std::forward<Args2>(args)...);
	}

	//bind a lambda
	template <typename LambdaType, typename... Args2>
	void bindLambda(LambdaType&& lambda, Args2&&... args)
	{
		*this = CreateLambda<LambdaType, Args2...>(std::forward<LambdaType>(lambda), std::forward<Args2>(args)...);
	}

	//bind a member function with a shared_ptr object
	template <typename T, typename... Args2>
	void bindSp(std::shared_ptr<T> pObject, NonConstMemberFunction<T, Args2...> pFunction, Args2&&... args)
	{
		DELEGATE_STATIC_ASSERT(!std::is_const<T>::value, "Cannot bind a non-const function on a const object");
		*this = CreateSP<T, Args2...>(pObject, pFunction, std::forward<Args2>(args)...);
	}

	template <typename T, typename... Args2>
	void bindSp(std::shared_ptr<T> pObject, ConstMemberFunction<T, Args2...> pFunction, Args2&&... args)
	{
		*this = CreateSP<T, Args2...>(pObject, pFunction, std::forward<Args2>(args)...);
	}

	//execute the delegate with the given parameters
	RetVal execute(Args... args) const
	{
		DELEGATE_ASSERT(m_allocator.hasAllocation(), "Delegate is not bound");
		return static_cast<IDelegateT*>(getDelegate())->execute(std::forward<Args>(args)...);
	}

	RetVal executeIfBound(Args... args) const
	{
		if (isBound())
		{
			return static_cast<IDelegateT*>(getDelegate())->execute(std::forward<Args>(args)...);
		}
		return RetVal();
	}

private:
	template <typename T, typename... Args3>
	void bind(Args3&&... args)
	{
		release();
		void* pAlloc = m_allocator.allocate(sizeof(T));
		new(pAlloc) T(std::forward<Args3>(args)...);
	}
};

//Delegate that can be bound to by MULTIPLE objects
template <typename... Args>
class MulticastDelegate : public DelegateBase
{
public:
	using DelegateT = Delegate<void, Args...>;

private:
	struct DelegateHandlerPair
	{
		DelegateHandle m_handle;
		DelegateT m_callback;

		DelegateHandlerPair() : m_handle(false) {}

		DelegateHandlerPair(const DelegateHandle& handle, const DelegateT& callback) : m_handle(handle),
			m_callback(callback) {}

		DelegateHandlerPair(const DelegateHandle& handle, DelegateT&& callback) : m_handle(handle),
			m_callback(std::move(callback)) {}
	};

	template <typename T, typename... Args2>
	using ConstMemberFunction = typename _DelegatesInternal::MemberFunction<true, T, void, Args..., Args2...>::Type;
	template <typename T, typename... Args2>
	using NonConstMemberFunction = typename _DelegatesInternal::MemberFunction<false, T, void, Args..., Args2...>::Type;

public:
	//Default constructor
	constexpr MulticastDelegate()
		: m_locks(0) {}

	//Default destructor
	~MulticastDelegate() noexcept override = default;

	//Default copy constructor
	MulticastDelegate(const MulticastDelegate& other) = default;

	//Default copy assignment operator
	MulticastDelegate& operator=(const MulticastDelegate& other) = default;

	//Move constructor
	MulticastDelegate(MulticastDelegate&& other) noexcept
		: m_events(std::move(other.m_events)),
		  m_locks(std::move(other.m_locks)) {}

	//Move assignment operator
	MulticastDelegate& operator=(MulticastDelegate&& other) noexcept
	{
		m_events = std::move(other.m_events);
		m_locks = std::move(other.m_locks);
		return *this;
	}

	template <typename T>
	DelegateHandle operator+=(T&& l)
	{
		return Add(DelegateT::CreateLambda(std::move(l)));
	}

	//Add delegate with the += operator
	DelegateHandle operator+=(DelegateT&& handler) noexcept
	{
		return Add(std::forward<DelegateT>(handler));
	}

	//remove a delegate using its DelegateHandle
	bool operator-=(DelegateHandle& handle)
	{
		return remove(handle);
	}

	DelegateHandle add(DelegateT&& handler) noexcept
	{
		//Favour an empty space over a possible array reallocation
		for (size_t i = 0; i < m_events.size(); ++i)
		{
			if (m_events[i].m_handle.isValid() == false)
			{
				m_events[i] = DelegateHandlerPair(DelegateHandle(true), std::move(handler));
				return m_events[i].m_handle;
			}
		}
		m_events.emplace_back(DelegateHandle(true), std::move(handler));
		return m_events.back().m_handle;
	}

	//bind a member function
	template <typename T, typename... Args2>
	DelegateHandle addRaw(T* pObject, NonConstMemberFunction<T, Args2...> pFunction, Args2&&... args)
	{
		return add(DelegateT::createRaw(pObject, pFunction, std::forward<Args2>(args)...));
	}

	template <typename T, typename... Args2>
	DelegateHandle addRaw(T* pObject, ConstMemberFunction<T, Args2...> pFunction, Args2&&... args)
	{
		return add(DelegateT::createRaw(pObject, pFunction, std::forward<Args2>(args)...));
	}

	//bind a static/global function
	template <typename... Args2>
	DelegateHandle addStatic(void (*pFunction)(Args..., Args2...), Args2&&... args)
	{
		return add(DelegateT::createStatic(pFunction, std::forward<Args2>(args)...));
	}

	//bind a lambda
	template <typename LambdaType, typename... Args2>
	DelegateHandle addLambda(LambdaType&& lambda, Args2&&... args)
	{
		return add(DelegateT::createLambda(std::forward<LambdaType>(lambda), std::forward<Args2>(args)...));
	}

	//bind a member function with a shared_ptr object
	template <typename T, typename... Args2>
	DelegateHandle addSp(std::shared_ptr<T> pObject, NonConstMemberFunction<T, Args2...> pFunction, Args2&&... args)
	{
		return add(DelegateT::createSP(pObject, pFunction, std::forward<Args2>(args)...));
	}

	template <typename T, typename... Args2>
	DelegateHandle addSp(std::shared_ptr<T> object, ConstMemberFunction<T, Args2...> function, Args2&&... args)
	{
		return add(DelegateT::createSP(object, function, std::forward<Args2>(args)...));
	}

	//Removes all handles that are bound from a specific object
	//Ignored when Object is null
	//Note: Only works on Raw and SP bindings
	void removeObject(void* object)
	{
		if (object != nullptr)
		{
			for (size_t index = 0; index < m_events.size(); ++index)
			{
				if (m_events[index].m_callback.getOwner() == object)
				{
					if (isLocked())
					{
						m_events[index].m_callback.Clear();
					}
					else
					{
						std::swap(m_events[index], m_events[m_events.size() - 1]);
						m_events.pop_back();
					}
				}
			}
		}
	}

	//remove a function from the event list by the m_handle
	bool remove(DelegateHandle& handle)
	{
		if (handle.isValid())
		{
			for (size_t i = 0; i < m_events.size(); ++i)
			{
				if (m_events[i].m_handle == handle)
				{
					if (isLocked())
					{
						m_events[i].m_callback.Clear();
					}
					else
					{
						std::swap(m_events[i], m_events[m_events.size() - 1]);
						m_events.pop_back();
					}
					handle.reset();
					return true;
				}
			}
		}
		return false;
	}

	bool isBoundTo(const DelegateHandle& handle) const
	{
		if (handle.isValid())
		{
			for (size_t index = 0; index < m_events.size(); ++index)
			{
				if (m_events[index].m_handle == handle)
				{
					return true;
				}
			}
		}
		return false;
	}

	//remove all the functions bound to the delegate
	void removeAll()
	{
		if (isLocked())
		{
			for (DelegateHandlerPair& handler : m_events)
			{
				handler.m_callback.Clear();
			}
		}
		else
		{
			m_events.clear();
		}
	}

	void compress(const size_t maxSpace = 0)
	{
		if (isLocked() == false)
		{
			size_t toDelete = 0;
			for (size_t i = 0; i < m_events.size() - toDelete; ++i)
			{
				if (m_events[i].m_handle.isValid() == false)
				{
					std::swap(m_events[i], m_events[toDelete]);
					++toDelete;
				}
			}
			if (toDelete > maxSpace)
			{
				m_events.resize(m_events.size() - toDelete);
			}
		}
	}

	//execute all functions that are bound
	void broadcast(Args... args)
	{
		lock();
		for (size_t i = 0; i < m_events.size(); ++i)
		{
			if (m_events[i].m_handle.isValid())
			{
				m_events[i].m_callback.execute(std::forward<Args>(args)...);
			}
		}
		unlock();
	}

	[[nodiscard]] size_t getSize() const
	{
		return m_events.size();
	}

private:
	void lock()
	{
		++m_locks;
	}

	void unlock()
	{
		//Unlock() should never be called more than Lock()!
		DELEGATE_ASSERT(m_locks > 0);
		--m_locks;
	}

	//Returns true is the delegate is currently broadcasting
	//If this is true, the order of the array should not be changed otherwise this causes undefined behaviour
	bool isLocked() const
	{
		return m_locks > 0;
	}

	std::vector<DelegateHandlerPair> m_events;
	unsigned int m_locks;
};
