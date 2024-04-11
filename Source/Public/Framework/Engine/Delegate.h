#pragma once

#include <vector>
#include <memory>
#include <tuple>

#ifndef DELEGATE_ASSERT
#include <assert.h>
#define DELEGATE_ASSERT(Expr, ...) assert(Expr)
#endif

#define DELEGATE_STATIC_ASSERT(Expr, Msg) static_assert(Expr, Msg)

//The allocation Size of delegate data.
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
	using MulticastDelegate::Broadcast; \
	using MulticastDelegate::RemoveAll; \
	using MulticastDelegate::Remove; \
};

///////////////////////////////////////////////////////////////
/////////////////// INTERNAL SECTION //////////////////////////
///////////////////////////////////////////////////////////////

#if __cplusplus >= 201703L
#define NO_DISCARD [[nodiscard]]
#else
#define NO_DISCARD		
#endif

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

    static void* (*Alloc)(size_t Size) = [](size_t Size) { return malloc(Size); };
    static void (*Free)(void* Ptr) = [](void* Ptr) { free(Ptr); };
    template <typename T>
    void DelegateDeleteFunc(T* Ptr)
    {
        Ptr->~T();
        DelegateFreeFunc(Ptr);
    }
}

namespace Delegates
{
    using AllocateCallback = void* (*)(size_t size);
    using FreeCallback = void(*)(void* pPtr);
    inline void SetAllocationCallbacks(AllocateCallback allocateCallback, FreeCallback freeCallback)
    {
        _DelegatesInternal::Alloc = allocateCallback;
        _DelegatesInternal::Free = freeCallback;
    }
}

class IDelegateBase
{
public:
    IDelegateBase() = default;
    virtual ~IDelegateBase() noexcept = default;
    virtual const void* GetOwner() const { return nullptr; }
    virtual void Clone(void* pDestination) = 0;
};

//Base type for delegates
template <typename RetVal, typename... Args>
class IDelegate : public IDelegateBase
{
public:
    virtual RetVal Execute(Args&&... args) = 0;
};

template <typename RetVal, typename... Args2>
class StaticDelegate;

template <typename RetVal, typename... Args, typename... Args2>
class StaticDelegate<RetVal(Args...), Args2...> : public IDelegate<RetVal, Args...>
{
public:
    using DelegateFunction = RetVal(*)(Args..., Args2...);

    StaticDelegate(DelegateFunction Func, Args2&&... payload)
        : m_Function(Func), m_Payload(std::forward<Args2>(payload)...)
    {
    }

    StaticDelegate(DelegateFunction function, const std::tuple<Args2...>& payload)
        : m_Function(function), m_Payload(payload)
    {
    }

    RetVal Execute(Args&&... args) override
    {
        return Execute_Internal(std::forward<Args>(args)..., std::index_sequence_for<Args2...>());
    }

    void Clone(void* pDestination) override
    {
        new(pDestination) StaticDelegate(m_Function, m_Payload);
    }

private:
    template <std::size_t... Is>
    RetVal Execute_Internal(Args&&... args, std::index_sequence<Is...>)
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
        : m_pObject(pObject), m_Function(function), m_Payload(std::forward<Args2>(payload)...)
    {
    }

    RawDelegate(T* pObject, DelegateFunction function, const std::tuple<Args2...>& payload)
        : m_pObject(pObject), m_Function(function), m_Payload(payload)
    {
    }

    RetVal Execute(Args&&... args) override
    {
        return Execute_Internal(std::forward<Args>(args)..., std::index_sequence_for<Args2...>());
    }
    const void* GetOwner() const override
    {
        return m_pObject;
    }

    void Clone(void* pDestination) override
    {
        new(pDestination) RawDelegate(m_pObject, m_Function, m_Payload);
    }

private:
    template <std::size_t... Is>
    RetVal Execute_Internal(Args&&... args, std::index_sequence<Is...>)
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
          m_Payload(std::forward<Args2>(payload)...)
    {
    }

    explicit LambdaDelegate(const TLambda& lambda, const std::tuple<Args2...>& payload)
        : m_Lambda(lambda),
          m_Payload(payload)
    {
    }

    RetVal Execute(Args&&... args) override
    {
        return Execute_Internal(std::forward<Args>(args)..., std::index_sequence_for<Args2...>());
    }

    void Clone(void* pDestination) override
    {
        new(pDestination) LambdaDelegate(m_Lambda, m_Payload);
    }

private:
    template <std::size_t... Is>
    RetVal Execute_Internal(Args&&... args, std::index_sequence<Is...>)
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
          m_Payload(std::forward<Args2>(payload)...)
    {
    }

    SPDelegate(std::weak_ptr<T> pObject, DelegateFunction pFunction, const std::tuple<Args2...>& payload)
        : m_pObject(pObject),
          m_pFunction(pFunction),
          m_Payload(payload)
    {
    }

    RetVal Execute(Args&&... args) override
    {
        return Execute_Internal(std::forward<Args>(args)..., std::index_sequence_for<Args2...>());
    }

    const void* GetOwner() const override
    {
        return m_pObject.expired() ? nullptr : m_pObject.lock().get();
    }

    void Clone(void* pDestination) override
    {
        new(pDestination) SPDelegate(m_pObject, m_pFunction, m_Payload);
    }

private:
    template <std::size_t... Is>
    RetVal Execute_Internal(Args&&... args, std::index_sequence<Is...>)
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

//A handle to a delegate used for a multicast delegate
//Static ID so that every handle is unique
class DelegateHandle
{
public:
    constexpr DelegateHandle() noexcept
        : m_Id(INVALID_ID)
    {
    }

    explicit DelegateHandle(bool /*generateId*/) noexcept
        : m_Id(GetNewID())
    {
    }

    ~DelegateHandle() noexcept = default;
    DelegateHandle(const DelegateHandle& other) = default;
    DelegateHandle& operator=(const DelegateHandle& other) = default;

    DelegateHandle(DelegateHandle&& other) noexcept
        : m_Id(other.m_Id)
    {
        other.Reset();
    }

    DelegateHandle& operator=(DelegateHandle&& other) noexcept
    {
        m_Id = other.m_Id;
        other.Reset();
        return *this;
    }

    operator bool() const noexcept
    {
        return IsValid();
    }

    bool operator==(const DelegateHandle& other) const noexcept
    {
        return m_Id == other.m_Id;
    }

    bool operator<(const DelegateHandle& other) const noexcept
    {
        return m_Id < other.m_Id;
    }

    bool IsValid() const noexcept
    {
        return m_Id != INVALID_ID;
    }

    void Reset() noexcept
    {
        m_Id = INVALID_ID;
    }

    constexpr static const unsigned int INVALID_ID = (unsigned int)~0;

private:
    unsigned int m_Id;
    inline static unsigned int CURRENT_ID = 0;

    static int GetNewID()
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
        : m_Size(0)
    {
        DELEGATE_STATIC_ASSERT(MaxStackSize > sizeof(void*), "MaxStackSize is smaller or equal to the Size of a pointer. This will make the use of an InlineAllocator pointless. Please increase the MaxStackSize.");
    }

    //Destructor
    ~InlineAllocator() noexcept
    {
        Free();
    }

    //Copy constructor
    InlineAllocator(const InlineAllocator& other)
        : m_Size(0)
    {
        if (other.HasAllocation())
        {
            memcpy(Allocate(other.m_Size), other.GetAllocation(), other.m_Size);
        }
        m_Size = other.m_Size;
    }

    //Copy assignment operator
    InlineAllocator& operator=(const InlineAllocator& other)
    {
        if (other.HasAllocation())
        {
            memcpy(Allocate(other.m_Size), other.GetAllocation(), other.m_Size);
        }
        m_Size = other.m_Size;
        return *this;
    }

    //Move constructor
    InlineAllocator(InlineAllocator&& other) noexcept
        : m_Size(other.m_Size)
    {
        other.m_Size = 0;
        if (m_Size > MaxStackSize)
        {
            std::swap(pPtr, other.pPtr);
        }
        else
        {
            memcpy(Buffer, other.Buffer, m_Size);
        }
    }

    //Move assignment operator
    InlineAllocator& operator=(InlineAllocator&& other) noexcept
    {
        Free();
        m_Size = other.m_Size;
        other.m_Size = 0;
        if (m_Size > MaxStackSize)
        {
            std::swap(pPtr, other.pPtr);
        }
        else
        {
            memcpy(Buffer, other.Buffer, m_Size);
        }
        return *this;
    }

    //Allocate memory of given Size
    //If the Size is over the predefined threshold, it will be allocated on the heap
    void* Allocate(const size_t size)
    {
        if (m_Size != size)
        {
            Free();
            m_Size = size;
            if (size > MaxStackSize)
            {
                pPtr = _DelegatesInternal::Alloc(size);
                return pPtr;
            }
        }
        return (void*)Buffer;
    }

    //Free the allocated memory
    void Free()
    {
        if (m_Size > MaxStackSize)
        {
            _DelegatesInternal::Free(pPtr);
        }
        m_Size = 0;
    }

    //Return the allocated memory either on the stack or on the heap
    void* GetAllocation() const
    {
        if (HasAllocation())
        {
            return HasHeapAllocation() ? pPtr : (void*)Buffer;
        }
        else
        {
            return nullptr;
        }
    }

    size_t GetSize() const
    {
        return m_Size;
    }

    bool HasAllocation() const
    {
        return m_Size > 0;
    }

    bool HasHeapAllocation() const
    {
        return m_Size > MaxStackSize;
    }

private:
    //If the allocation is smaller than the threshold, Buffer is used
    //Otherwise Ptr is used together with a separate dynamic allocation
    union
    {
        char Buffer[MaxStackSize];
        void* pPtr;
    };
    size_t m_Size;
};

class DelegateBase
{
public:
    //Default constructor
    constexpr DelegateBase() noexcept
    {
    }

    //Default destructor
    virtual ~DelegateBase() noexcept
    {
        Release();
    }

    //Copy contructor
    DelegateBase(const DelegateBase& other)
    {
        if (other.m_Allocator.HasAllocation())
        {
            m_Allocator.Allocate(other.m_Allocator.GetSize());
            other.GetDelegate()->Clone(m_Allocator.GetAllocation());
        }
    }

    //Copy assignment operator
    DelegateBase& operator=(const DelegateBase& other)
    {
        Release();
        if (other.m_Allocator.HasAllocation())
        {
            m_Allocator.Allocate(other.m_Allocator.GetSize());
            other.GetDelegate()->Clone(m_Allocator.GetAllocation());
        }
        return *this;
    }

    //Move constructor
    DelegateBase(DelegateBase&& other) noexcept
        : m_Allocator(std::move(other.m_Allocator))
    {
    }

    //Move assignment operator
    DelegateBase& operator=(DelegateBase&& other) noexcept
    {
        Release();
        m_Allocator = std::move(other.m_Allocator);
        return *this;
    }

    //Gets the owner of the deletage
    //Only valid for SPDelegate and RawDelegate.
    //Otherwise returns nullptr by default
    const void* GetOwner() const
    {
        if (m_Allocator.HasAllocation())
        {
            return GetDelegate()->GetOwner();
        }
        return nullptr;
    }

    size_t GetSize() const
    {
        return m_Allocator.GetSize();
    }

    //Clear the bound delegate if it is bound to the given object.
    //Ignored when pObject is a nullptr
    void ClearIfBoundTo(void* pObject)
    {
        if (pObject != nullptr && IsBoundTo(pObject))
        {
            Release();
        }
    }

    //Clear the bound delegate if it exists
    void Clear()
    {
        Release();
    }

    //If the allocator has a Size, it means it's bound to something
    bool IsBound() const
    {
        return m_Allocator.HasAllocation();
    }

    bool IsBoundTo(void* pObject) const
    {
        if (pObject == nullptr || m_Allocator.HasAllocation() == false)
        {
            return false;
        }
        return GetDelegate()->GetOwner() == pObject;
    }

protected:
    void Release()
    {
        if (m_Allocator.HasAllocation())
        {
            GetDelegate()->~IDelegateBase();
            m_Allocator.Free();
        }
    }

    IDelegateBase* GetDelegate() const
    {
        return static_cast<IDelegateBase*>(m_Allocator.GetAllocation());
    }

    //Allocator for the delegate itself.
    //Delegate gets allocated when its is smaller or equal than 64 bytes in Size.
    //Can be changed by preference
    InlineAllocator<DELEGATE_INLINE_ALLOCATION_SIZE> m_Allocator;
};

//Delegate that can be bound to by just ONE object
template <typename RetVal, typename... Args>
class Delegate : public DelegateBase
{
private:
    template <typename T, typename... Args2>
    using ConstMemberFunction = typename _DelegatesInternal::MemberFunction<true, T, RetVal, Args..., Args2...>::Type;
    template <typename T, typename... Args2>
    using NonConstMemberFunction = typename _DelegatesInternal::MemberFunction<false, T, RetVal, Args..., Args2...>::Type;

public:
    using IDelegateT = IDelegate<RetVal, Args...>;

    //Create delegate using member function
    template <typename T, typename... Args2>
    NO_DISCARD static Delegate CreateRaw(T* pObj, NonConstMemberFunction<T, Args2...> pFunction, Args2... args)
    {
        Delegate handler;
        handler.Bind<RawDelegate<false, T, RetVal(Args...), Args2...>>(pObj, pFunction, std::forward<Args2>(args)...);
        return handler;
    }

    template <typename T, typename... Args2>
    NO_DISCARD static Delegate CreateRaw(T* pObj, ConstMemberFunction<T, Args2...> pFunction, Args2... args)
    {
        Delegate handler;
        handler.Bind<RawDelegate<true, T, RetVal(Args...), Args2...>>(pObj, pFunction, std::forward<Args2>(args)...);
        return handler;
    }

    //Create delegate using global/static function
    template <typename... Args2>
    NO_DISCARD static Delegate CreateStatic(RetVal (*pFunction)(Args..., Args2...), Args2... args)
    {
        Delegate handler;
        handler.Bind<StaticDelegate<RetVal(Args...), Args2...>>(pFunction, std::forward<Args2>(args)...);
        return handler;
    }

    //Create delegate using std::shared_ptr
    template <typename T, typename... Args2>
    NO_DISCARD static Delegate CreateSP(const std::shared_ptr<T>& pObject, NonConstMemberFunction<T, Args2...> pFunction, Args2... args)
    {
        Delegate handler;
        handler.Bind<SPDelegate<false, T, RetVal(Args...), Args2...>>(pObject, pFunction, std::forward<Args2>(args)...);
        return handler;
    }

    template <typename T, typename... Args2>
    NO_DISCARD static Delegate CreateSP(const std::shared_ptr<T>& pObject, ConstMemberFunction<T, Args2...> pFunction, Args2... args)
    {
        Delegate handler;
        handler.Bind<SPDelegate<true, T, RetVal(Args...), Args2...>>(pObject, pFunction, std::forward<Args2>(args)...);
        return handler;
    }

    //Create delegate using a lambda
    template <typename TLambda, typename... Args2>
    NO_DISCARD static Delegate CreateLambda(TLambda&& lambda, Args2... args)
    {
        Delegate handler;
        handler.Bind<LambdaDelegate<TLambda, RetVal(Args...), Args2...>>(std::forward<TLambda>(lambda), std::forward<Args2>(args)...);
        return handler;
    }

    //Bind a member function
    template <typename T, typename... Args2>
    void BindRaw(T* pObject, NonConstMemberFunction<T, Args2...> pFunction, Args2&&... args)
    {
        DELEGATE_STATIC_ASSERT(!std::is_const<T>::value, "Cannot bind a non-const function on a const object");
        *this = CreateRaw<T, Args2...>(pObject, pFunction, std::forward<Args2>(args)...);
    }

    template <typename T, typename... Args2>
    void BindRaw(T* pObject, ConstMemberFunction<T, Args2...> pFunction, Args2&&... args)
    {
        *this = CreateRaw<T, Args2...>(pObject, pFunction, std::forward<Args2>(args)...);
    }

    //Bind a static/global function
    template <typename... Args2>
    void BindStatic(RetVal (*pFunction)(Args..., Args2...), Args2&&... args)
    {
        *this = CreateStatic<Args2...>(pFunction, std::forward<Args2>(args)...);
    }

    //Bind a lambda
    template <typename LambdaType, typename... Args2>
    void BindLambda(LambdaType&& lambda, Args2&&... args)
    {
        *this = CreateLambda<LambdaType, Args2...>(std::forward<LambdaType>(lambda), std::forward<Args2>(args)...);
    }

    //Bind a member function with a shared_ptr object
    template <typename T, typename... Args2>
    void BindSP(std::shared_ptr<T> pObject, NonConstMemberFunction<T, Args2...> pFunction, Args2&&... args)
    {
        DELEGATE_STATIC_ASSERT(!std::is_const<T>::value, "Cannot bind a non-const function on a const object");
        *this = CreateSP<T, Args2...>(pObject, pFunction, std::forward<Args2>(args)...);
    }

    template <typename T, typename... Args2>
    void BindSP(std::shared_ptr<T> pObject, ConstMemberFunction<T, Args2...> pFunction, Args2&&... args)
    {
        *this = CreateSP<T, Args2...>(pObject, pFunction, std::forward<Args2>(args)...);
    }

    //Execute the delegate with the given parameters
    RetVal Execute(Args... args) const
    {
        DELEGATE_ASSERT(m_Allocator.HasAllocation(), "Delegate is not bound");
        return ((IDelegateT*)GetDelegate())->Execute(std::forward<Args>(args)...);
    }

    RetVal ExecuteIfBound(Args... args) const
    {
        if (IsBound())
        {
            return ((IDelegateT*)GetDelegate())->Execute(std::forward<Args>(args)...);
        }
        return RetVal();
    }

private:
    template <typename T, typename... Args3>
    void Bind(Args3&&... args)
    {
        Release();
        void* pAlloc = m_Allocator.Allocate(sizeof(T));
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
        DelegateHandle Handle;
        DelegateT Callback;
        DelegateHandlerPair() : Handle(false)
        {
        }
        DelegateHandlerPair(const DelegateHandle& handle, const DelegateT& callback) : Handle(handle), Callback(callback)
        {
        }
        DelegateHandlerPair(const DelegateHandle& handle, DelegateT&& callback) : Handle(handle), Callback(std::move(callback))
        {
        }
    };
    template <typename T, typename... Args2>
    using ConstMemberFunction = typename _DelegatesInternal::MemberFunction<true, T, void, Args..., Args2...>::Type;
    template <typename T, typename... Args2>
    using NonConstMemberFunction = typename _DelegatesInternal::MemberFunction<false, T, void, Args..., Args2...>::Type;

public:
    //Default constructor
    constexpr MulticastDelegate()
        : Locks(0)
    {
    }

    //Default destructor
    ~MulticastDelegate() noexcept override = default;

    //Default copy constructor
    MulticastDelegate(const MulticastDelegate& other) = default;

    //Defaul copy assignment operator
    MulticastDelegate& operator=(const MulticastDelegate& other) = default;

    //Move constructor
    MulticastDelegate(MulticastDelegate&& other) noexcept
        : Events(std::move(other.Events)),
          Locks(std::move(other.Locks))
    {
    }

    //Move assignment operator
    MulticastDelegate& operator=(MulticastDelegate&& other) noexcept
    {
        Events = std::move(other.Events);
        Locks = std::move(other.Locks);
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

    //Remove a delegate using its DelegateHandle
    bool operator-=(DelegateHandle& handle)
    {
        return Remove(handle);
    }

    DelegateHandle Add(DelegateT&& handler) noexcept
    {
        //Favour an empty space over a possible array reallocation
        for (size_t i = 0; i < Events.size(); ++i)
        {
            if (Events[i].Handle.IsValid() == false)
            {
                Events[i] = DelegateHandlerPair(DelegateHandle(true), std::move(handler));
                return Events[i].Handle;
            }
        }
        Events.emplace_back(DelegateHandle(true), std::move(handler));
        return Events.back().Handle;
    }

    //Bind a member function
    template <typename T, typename... Args2>
    DelegateHandle AddRaw(T* pObject, NonConstMemberFunction<T, Args2...> pFunction, Args2&&... args)
    {
        return Add(DelegateT::CreateRaw(pObject, pFunction, std::forward<Args2>(args)...));
    }

    template <typename T, typename... Args2>
    DelegateHandle AddRaw(T* pObject, ConstMemberFunction<T, Args2...> pFunction, Args2&&... args)
    {
        return Add(DelegateT::CreateRaw(pObject, pFunction, std::forward<Args2>(args)...));
    }

    //Bind a static/global function
    template <typename... Args2>
    DelegateHandle AddStatic(void (*pFunction)(Args..., Args2...), Args2&&... args)
    {
        return Add(DelegateT::CreateStatic(pFunction, std::forward<Args2>(args)...));
    }

    //Bind a lambda
    template <typename LambdaType, typename... Args2>
    DelegateHandle AddLambda(LambdaType&& lambda, Args2&&... args)
    {
        return Add(DelegateT::CreateLambda(std::forward<LambdaType>(lambda), std::forward<Args2>(args)...));
    }

    //Bind a member function with a shared_ptr object
    template <typename T, typename... Args2>
    DelegateHandle AddSP(std::shared_ptr<T> pObject, NonConstMemberFunction<T, Args2...> pFunction, Args2&&... args)
    {
        return Add(DelegateT::CreateSP(pObject, pFunction, std::forward<Args2>(args)...));
    }

    template <typename T, typename... Args2>
    DelegateHandle AddSP(std::shared_ptr<T> Object, ConstMemberFunction<T, Args2...> Function, Args2&&... args)
    {
        return Add(DelegateT::CreateSP(Object, Function, std::forward<Args2>(args)...));
    }

    //Removes all handles that are bound from a specific object
    //Ignored when Object is null
    //Note: Only works on Raw and SP bindings
    void RemoveObject(void* Object)
    {
        if (Object != nullptr)
        {
            for (size_t Index = 0; Index < Events.size(); ++Index)
            {
                if (Events[Index].Callback.GetOwner() == Object)
                {
                    if (IsLocked())
                    {
                        Events[Index].Callback.Clear();
                    }
                    else
                    {
                        std::swap(Events[Index], Events[Events.size() - 1]);
                        Events.pop_back();
                    }
                }
            }
        }
    }

    //Remove a function from the event list by the Handle
    bool Remove(DelegateHandle& Handle)
    {
        if (Handle.IsValid())
        {
            for (size_t i = 0; i < Events.size(); ++i)
            {
                if (Events[i].Handle == Handle)
                {
                    if (IsLocked())
                    {
                        Events[i].Callback.Clear();
                    }
                    else
                    {
                        std::swap(Events[i], Events[Events.size() - 1]);
                        Events.pop_back();
                    }
                    Handle.Reset();
                    return true;
                }
            }
        }
        return false;
    }

    bool IsBoundTo(const DelegateHandle& Handle) const
    {
        if (Handle.IsValid())
        {
            for (size_t Index = 0; Index < Events.size(); ++Index)
            {
                if (Events[Index].Handle == Handle)
                {
                    return true;
                }
            }
        }
        return false;
    }

    //Remove all the functions bound to the delegate
    void RemoveAll()
    {
        if (IsLocked())
        {
            for (DelegateHandlerPair& Handler : Events)
            {
                Handler.Callback.Clear();
            }
        }
        else
        {
            Events.clear();
        }
    }

    void Compress(const size_t MaxSpace = 0)
    {
        if (IsLocked() == false)
        {
            size_t ToDelete = 0;
            for (size_t i = 0; i < Events.size() - ToDelete; ++i)
            {
                if (Events[i].Handle.IsValid() == false)
                {
                    std::swap(Events[i], Events[ToDelete]);
                    ++ToDelete;
                }
            }
            if (ToDelete > MaxSpace)
            {
                Events.resize(Events.size() - ToDelete);
            }
        }
    }

    //Execute all functions that are bound
    void Broadcast(Args... args)
    {
        Lock();
        for (size_t i = 0; i < Events.size(); ++i)
        {
            if (Events[i].Handle.IsValid())
            {
                Events[i].Callback.Execute(std::forward<Args>(args)...);
            }
        }
        Unlock();
    }

    size_t GetSize() const
    {
        return Events.size();
    }

private:
    void Lock()
    {
        ++Locks;
    }

    void Unlock()
    {
        //Unlock() should never be called more than Lock()!
        DELEGATE_ASSERT(m_Locks > 0);
        --Locks;
    }

    //Returns true is the delegate is currently broadcasting
    //If this is true, the order of the array should not be changed otherwise this causes undefined behaviour
    bool IsLocked() const
    {
        return Locks > 0;
    }

    std::vector<DelegateHandlerPair> Events;
    unsigned int Locks;
};
