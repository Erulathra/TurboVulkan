#pragma once

// Original Source: https://github.com/simco50/CppDelegates

#include <vector>
#include <memory>
#include <ranges>
#include <tuple>

//The allocation size of delegate data.
//Delegates larger than this will be heap allocated.
#ifndef DELEGATE_INLINE_ALLOCATION_SIZE
#define DELEGATE_INLINE_ALLOCATION_SIZE 32
#endif

#define DECLARE_DELEGATE(name, ...) \
using name = TDelegate<void __VA_OPT__(,) __VA_ARGS__>

#define DECLARE_DELEGATE_RET(name, retValue, ...) \
using name = TDelegate<retValue __VA_OPT__(,) __VA_ARGS__>

#define DECLARE_MULTICAST_DELEGATE(name, ...) \
using name = TMulticastDelegate<EDelegateExecutionOrder::Undefined __VA_OPT__(,) __VA_ARGS__>

#define DECLARE_MULTICAST_DELEGATE_REVERSE(name, ...) \
using name = TMulticastDelegate<EDelegateExecutionOrder::Reverse __VA_OPT__(,) __VA_ARGS__>

#define DECLARE_EVENT(name, ownerType, ...) \
class name final : public TMulticastDelegate<EDelegateExecutionOrder::Undefined __VA_OPT__(,) __VA_ARGS__> \
{ \
private: \
friend class ownerType; \
using TMulticastDelegate::Broadcast; \
using TMulticastDelegate::RemoveAll; \
using TMulticastDelegate::Remove; \
};

namespace Turbo
{
	namespace _DelegatesInternal
	{
		template <bool IsConst, typename Object, typename RetVal, typename... Args>
		struct TMemberFunction;

		template <typename Object, typename RetVal, typename... Args>
		struct TMemberFunction<true, Object, RetVal, Args...>
		{
			using Type = RetVal(Object::*)(Args...) const;
		};

		template <typename Object, typename RetVal, typename... Args>
		struct TMemberFunction<false, Object, RetVal, Args...>
		{
			using Type = RetVal(Object::*)(Args...);
		};

		static void* (*Alloc)(size_t size) = [](size_t size) { return malloc(size); };
		static void (*Free)(void* pPtr) = [](void* pPtr) { free(pPtr); };
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
		[[nodiscard]] virtual const void* GetOwner() const { return nullptr; }

		virtual void Clone(void* pDestination) = 0;
	};

	/** Base type for delegates */
	template <typename RetVal, typename... Args>
	class IDelegate : public IDelegateBase
	{
	public:
		virtual RetVal Execute(Args&&... args) = 0;
	};

	template <typename RetVal, typename... Args2>
	class TStaticDelegate;

	template <typename RetVal, typename... Args, typename... Args2>
	class TStaticDelegate<RetVal(Args...), Args2...> final : public IDelegate<RetVal, Args...>
	{
	public:
		using DelegateFunction = RetVal(*)(Args..., Args2...);

		explicit TStaticDelegate(DelegateFunction function, Args2&&... payload)
			: mFunction(function), mPayload(std::forward<Args2>(payload)...)
		{
		}

		TStaticDelegate(DelegateFunction function, const std::tuple<Args2...>& payload)
			: mFunction(function), mPayload(payload)
		{
		}

		virtual RetVal Execute(Args&&... args) override
		{
			return Execute_Internal(std::forward<Args>(args)..., std::index_sequence_for<Args2...>());
		}

		virtual void Clone(void* pDestination) override
		{
			new(pDestination) TStaticDelegate(mFunction, mPayload);
		}

	private:
		template <std::size_t... Is>
		RetVal Execute_Internal(Args&&... args, std::index_sequence<Is...>)
		{
			return mFunction(std::forward<Args>(args)..., std::get<Is>(mPayload)...);
		}

		DelegateFunction mFunction;
		std::tuple<Args2...> mPayload;
	};

	template <bool IsConst, typename T, typename RetVal, typename... Args2>
	class TRawDelegate;

	template <bool IsConst, typename T, typename RetVal, typename... Args, typename... Args2>
	class TRawDelegate<IsConst, T, RetVal(Args...), Args2...> final : public IDelegate<RetVal, Args...>
	{
	public:
		using DelegateFunction = typename _DelegatesInternal::TMemberFunction<IsConst, T, RetVal, Args..., Args2...>::Type;

		TRawDelegate(T* pObject, DelegateFunction function, Args2&&... payload)
			: mObjectPtr(pObject), mFunctionPtr(function), mPayload(std::forward<Args2>(payload)...)
		{
		}

		TRawDelegate(T* pObject, DelegateFunction function, const std::tuple<Args2...>& payload)
			: mObjectPtr(pObject), mFunctionPtr(function), mPayload(payload)
		{
		}

		virtual RetVal Execute(Args&&... args) override
		{
			return Execute_Internal(std::forward<Args>(args)..., std::index_sequence_for<Args2...>());
		}

		virtual const void* GetOwner() const override
		{
			return mObjectPtr;
		}

		virtual void Clone(void* pDestination) override
		{
			new(pDestination) TRawDelegate(mObjectPtr, mFunctionPtr, mPayload);
		}

	private:
		template <std::size_t... Is>
		RetVal Execute_Internal(Args&&... args, std::index_sequence<Is...>)
		{
			return (mObjectPtr->*mFunctionPtr)(std::forward<Args>(args)..., std::get<Is>(mPayload)...);
		}

		T* mObjectPtr;
		DelegateFunction mFunctionPtr;
		std::tuple<Args2...> mPayload;
	};

	template <typename TLambda, typename RetVal, typename... Args>
	class TLambdaDelegate;

	template <typename TLambda, typename RetVal, typename... Args, typename... Args2>
	class TLambdaDelegate<TLambda, RetVal(Args...), Args2...> final : public IDelegate<RetVal, Args...>
	{
	public:
		explicit TLambdaDelegate(TLambda&& lambda, Args2&&... payload)
			: mLambda(std::forward<TLambda>(lambda)),
			  mPayload(std::forward<Args2>(payload)...)
		{
		}

		explicit TLambdaDelegate(const TLambda& lambda, const std::tuple<Args2...>& payload)
			: mLambda(lambda),
			  mPayload(payload)
		{
		}

		RetVal Execute(Args&&... args) override
		{
			return Execute_Internal(std::forward<Args>(args)..., std::index_sequence_for<Args2...>());
		}

		virtual void Clone(void* pDestination) override
		{
			new(pDestination) TLambdaDelegate(mLambda, mPayload);
		}

	private:
		template <std::size_t... Is>
		RetVal Execute_Internal(Args&&... args, std::index_sequence<Is...>)
		{
			return static_cast<RetVal>((mLambda)(std::forward<Args>(args)..., std::get<Is>(mPayload)...));
		}

		TLambda mLambda;
		std::tuple<Args2...> mPayload;
	};

	template <bool IsConst, typename T, typename RetVal, typename... Args>
	class TSPDelegate;

	template <bool IsConst, typename RetVal, typename T, typename... Args, typename... Args2>
	class TSPDelegate<IsConst, T, RetVal(Args...), Args2...> final : public IDelegate<RetVal, Args...>
	{
	public:
		using DelegateFunction = typename _DelegatesInternal::TMemberFunction<IsConst, T, RetVal, Args..., Args2...>::Type;

		TSPDelegate(std::shared_ptr<T> objectPtr, DelegateFunction functionPtr, Args2&&... payload)
			: mObjectPtr(objectPtr),
			  mFunctionPtr(functionPtr),
			  mPayload(std::forward<Args2>(payload)...)
		{
		}

		TSPDelegate(std::weak_ptr<T> objectPtr, DelegateFunction functionPtr, const std::tuple<Args2...>& payload)
			: mObjectPtr(objectPtr),
			  mFunctionPtr(functionPtr),
			  mPayload(payload)
		{
		}

		virtual RetVal Execute(Args&&... args) override
		{
			return Execute_Internal(std::forward<Args>(args)..., std::index_sequence_for<Args2...>());
		}

		virtual const void* GetOwner() const override
		{
			return mObjectPtr.expired() ? nullptr : mObjectPtr.lock().get();
		}

		virtual void Clone(void* destinationPtr) override
		{
			new (destinationPtr) TSPDelegate(mObjectPtr, mFunctionPtr, mPayload);
		}

	private:
		template <std::size_t... Is>
		RetVal Execute_Internal(Args&&... args, std::index_sequence<Is...>)
		{
			if (mObjectPtr.expired())
			{
				return RetVal();
			}
			else
			{
				std::shared_ptr<T> pPinned = mObjectPtr.lock();
				return (pPinned.get()->*mFunctionPtr)(std::forward<Args>(args)..., std::get<Is>(mPayload)...);
			}
		}

		std::weak_ptr<T> mObjectPtr;
		DelegateFunction mFunctionPtr;
		std::tuple<Args2...> mPayload;
	};

	//A handle to a delegate used for a multicast delegate
	//Static ID so that every handle is unique
	class FDelegateHandle
	{
	public:
		using FDelegateID = uint32;
		static constexpr FDelegateID INVALID_ID = 0;
	public:
		constexpr FDelegateHandle() noexcept
			: mId(INVALID_ID)
		{
		}

		explicit FDelegateHandle(bool /*generateId*/) noexcept
			: mId(GetNewID())
		{
		}

		~FDelegateHandle() noexcept = default;
		FDelegateHandle(const FDelegateHandle& other) = default;
		FDelegateHandle& operator=(const FDelegateHandle& other) = default;

		FDelegateHandle(FDelegateHandle&& other) noexcept
			: mId(other.mId)
		{
			other.Reset();
		}

		FDelegateHandle& operator=(FDelegateHandle&& other) noexcept
		{
			mId = other.mId;
			other.Reset();
			return *this;
		}

		operator bool() const noexcept
		{
			return IsValid();
		}

		bool operator==(const FDelegateHandle& other) const noexcept
		{
			return mId == other.mId;
		}

		bool operator<(const FDelegateHandle& other) const noexcept
		{
			return mId < other.mId;
		}

		bool IsValid() const noexcept
		{
			return mId != INDEX_NONE;
		}

		void Reset() noexcept
		{
			mId = INDEX_NONE;
		}

	private:
		FDelegateID mId;
		static FDelegateID sCurrentId;

		static FDelegateID GetNewID()
		{
			const FDelegateID output = sCurrentId++;
			if (sCurrentId == INDEX_NONE)
			{
				sCurrentId = 0;
			}
			return output;
		}
	};

	template <size_t MaxStackSize>
	class FInlineAllocator
	{
	public:
		//Constructor
		constexpr FInlineAllocator() noexcept
			: mPtr(nullptr)
			, mSize(0)
		{
			TURBO_STATIC_ASSERT_MSG(MaxStackSize > sizeof(void*), "MaxStackSize is smaller or equal to the size of a pointer. This will make the use of an InlineAllocator pointless. Please increase the MaxStackSize.");
		}

		//Destructor
		~FInlineAllocator() noexcept
		{
			Free();
		}

		//Copy constructor
		FInlineAllocator(const FInlineAllocator& other)
			: mSize(0)
		{
			if (other.HasAllocation())
			{
				memcpy(Allocate(other.mSize), other.GetAllocation(), other.mSize);
			}
			mSize = other.mSize;
		}

		//Copy assignment operator
		FInlineAllocator& operator=(const FInlineAllocator& other)
		{
			if (other.HasAllocation())
			{
				memcpy(Allocate(other.mSize), other.GetAllocation(), other.mSize);
			}
			mSize = other.mSize;
			return *this;
		}

		//Move constructor
		FInlineAllocator(FInlineAllocator&& other) noexcept
			: mSize(other.mSize)
		{
			other.mSize = 0;
			if (mSize > MaxStackSize)
			{
				std::swap(mPtr, other.mPtr);
			}
			else
			{
				memcpy(mBuffer, other.mBuffer, mSize);
			}
		}

		//Move assignment operator
		FInlineAllocator& operator=(FInlineAllocator&& other) noexcept
		{
			Free();
			mSize = other.mSize;
			other.mSize = 0;
			if (mSize > MaxStackSize)
			{
				std::swap(mPtr, other.mPtr);
			}
			else
			{
				memcpy(mBuffer, other.mBuffer, mSize);
			}
			return *this;
		}

		//Allocate memory of given size
		//If the size is over the predefined threshold, it will be allocated on the heap
		void* Allocate(const size_t size)
		{
			if (mSize != size)
			{
				Free();
				mSize = size;
				if (size > MaxStackSize)
				{
					mPtr = _DelegatesInternal::Alloc(size);
					return mPtr;
				}
			}
			return (void*)mBuffer;
		}

		//Free the allocated memory
		void Free()
		{
			if (mSize > MaxStackSize)
			{
				_DelegatesInternal::Free(mPtr);
			}
			mSize = 0;
		}

		//Return the allocated memory either on the stack or on the heap
		[[nodiscard]] void* GetAllocation() const
		{
			if (HasAllocation())
			{
				return HasHeapAllocation() ? mPtr : (void*)(mBuffer);
			}
			else
			{
				return nullptr;
			}
		}

		size_t GetSize() const
		{
			return mSize;
		}

		bool HasAllocation() const
		{
			return mSize > 0;
		}

		bool HasHeapAllocation() const
		{
			return mSize > MaxStackSize;
		}

	private:
		//If the allocation is smaller than the threshold, Buffer is used
		//Otherwise pPtr is used together with a separate dynamic allocation
		union
		{
			uint8 mBuffer[MaxStackSize];
			void* mPtr;
		};

		size_t mSize;
	};

	class FDelegateBase
	{
		GENERATED_BODY_MINIMAL(FDelegateBase)

	public:
		//Default constructor
		constexpr FDelegateBase() noexcept
			: mAllocator()
		{
		}

		//Default destructor
		virtual ~FDelegateBase() noexcept
		{
			Release();
		}

		//Copy constructor
		FDelegateBase(const FDelegateBase& other)
		{
			if (other.mAllocator.HasAllocation())
			{
				mAllocator.Allocate(other.mAllocator.GetSize());
				other.GetDelegate()->Clone(mAllocator.GetAllocation());
			}
		}

		//Copy assignment operator
		FDelegateBase& operator=(const FDelegateBase& other)
		{
			Release();
			if (other.mAllocator.HasAllocation())
			{
				mAllocator.Allocate(other.mAllocator.GetSize());
				other.GetDelegate()->Clone(mAllocator.GetAllocation());
			}
			return *this;
		}

		//Move constructor
		FDelegateBase(FDelegateBase&& other) noexcept
			: mAllocator(std::move(other.mAllocator))
		{
		}

		//Move assignment operator
		FDelegateBase& operator=(FDelegateBase&& other) noexcept
		{
			Release();
			mAllocator = std::move(other.mAllocator);
			return *this;
		}

		//Gets the owner of the delegate
		//Only valid for SPDelegate and RawDelegate.
		//Otherwise, returns nullptr by default
		const void* GetOwner() const
		{
			if (mAllocator.HasAllocation())
			{
				return GetDelegate()->GetOwner();
			}
			return nullptr;
		}

		virtual size_t GetSize() const
		{
			return mAllocator.GetSize();
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
		virtual void Clear()
		{
			Release();
		}

		//If the allocator has a size, it means it's bound to something
		bool IsBound() const
		{
			return mAllocator.HasAllocation();
		}

		bool IsBoundTo(void* pObject) const
		{
			if (pObject == nullptr || mAllocator.HasAllocation() == false)
			{
				return false;
			}
			return GetDelegate()->GetOwner() == pObject;
		}

	protected:
		void Release()
		{
			if (mAllocator.HasAllocation())
			{
				GetDelegate()->~IDelegateBase();
				mAllocator.Free();
			}
		}

		IDelegateBase* GetDelegate() const
		{
			return static_cast<IDelegateBase*>(mAllocator.GetAllocation());
		}

		//Allocator for the delegate itself.
		//Delegate gets allocated when its is smaller or equal than 64 bytes in size.
		//Can be changed by preference
		FInlineAllocator<DELEGATE_INLINE_ALLOCATION_SIZE> mAllocator;
	};

	//Delegate that can be bound to by just ONE object
	template <typename RetVal, typename... Args>
	class TDelegate final : public FDelegateBase
	{
	private:
		template <typename T, typename... Args2>
		using ConstMemberFunction = typename _DelegatesInternal::TMemberFunction<true, T, RetVal, Args..., Args2...>::Type;
		template <typename T, typename... Args2>
		using NonConstMemberFunction = typename _DelegatesInternal::TMemberFunction<false, T, RetVal, Args..., Args2...>::Type;

	public:
		using IDelegateT = IDelegate<RetVal, Args...>;

		//Create delegate using member function
		template <typename T, typename... Args2>
		[[nodiscard]] static TDelegate CreateRaw(T* pObj, NonConstMemberFunction<T, Args2...> pFunction, Args2... args)
		{
			TDelegate handler;
			handler.Bind<TRawDelegate<false, T, RetVal(Args...), Args2...>>(pObj, pFunction, std::forward<Args2>(args)...);
			return handler;
		}

		template <typename T, typename... Args2>
		[[nodiscard]] static TDelegate CreateRaw(T* pObj, ConstMemberFunction<T, Args2...> pFunction, Args2... args)
		{
			TDelegate handler;
			handler.Bind<TRawDelegate<true, T, RetVal(Args...), Args2...>>(pObj, pFunction, std::forward<Args2>(args)...);
			return handler;
		}

		//Create delegate using global/static function
		template <typename... Args2>
		[[nodiscard]] static TDelegate CreateStatic(RetVal (*pFunction)(Args..., Args2...), Args2... args)
		{
			TDelegate handler;
			handler.Bind<TStaticDelegate<RetVal(Args...), Args2...>>(pFunction, std::forward<Args2>(args)...);
			return handler;
		}

		//Create delegate using std::shared_ptr
		template <typename T, typename... Args2>
		[[nodiscard]] static TDelegate CreateSP(const std::shared_ptr<T>& pObject, NonConstMemberFunction<T, Args2...> pFunction, Args2... args)
		{
			TDelegate handler;
			handler.Bind<TSPDelegate<false, T, RetVal(Args...), Args2...>>(pObject, pFunction, std::forward<Args2>(args)...);
			return handler;
		}

		template <typename T, typename... Args2>
		[[nodiscard]] static TDelegate CreateSP(const std::shared_ptr<T>& pObject, ConstMemberFunction<T, Args2...> pFunction, Args2... args)
		{
			TDelegate handler;
			handler.Bind<TSPDelegate<true, T, RetVal(Args...), Args2...>>(pObject, pFunction, std::forward<Args2>(args)...);
			return handler;
		}

		//Create delegate using a lambda
		template <typename TLambda, typename... Args2>
		[[nodiscard]] static TDelegate CreateLambda(TLambda&& lambda, Args2... args)
		{
			TDelegate handler;
			using LambdaType = std::decay_t<TLambda>;
			handler.Bind<TLambdaDelegate<LambdaType, RetVal(Args...), Args2...>>(std::forward<LambdaType>(lambda), std::forward<Args2>(args)...);
			return handler;
		}

		//Bind a member function
		template <typename T, typename... Args2> requires (!std::is_const_v<T>)
		void BindRaw(T* pObject, NonConstMemberFunction<T, Args2...> pFunction, Args2&&... args)
		{
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
		template <typename T, typename... Args2> requires (!std::is_const_v<T>)
		void BindSP(std::shared_ptr<T> pObject, NonConstMemberFunction<T, Args2...> pFunction, Args2&&... args)
		{
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
			TURBO_CHECK_MSG(mAllocator.HasAllocation(), "Delegate is not bound");
			return static_cast<IDelegateT*>(GetDelegate())->Execute(std::forward<Args>(args)...);
		}

		RetVal ExecuteIfBound(Args... args) const
		{
			if (IsBound())
			{
				return static_cast<IDelegateT*>(GetDelegate())->Execute(std::forward<Args>(args)...);
			}
			return RetVal();
		}

	private:
		template <typename T, typename... Args3>
		void Bind(Args3&&... args)
		{
			Release();
			void* pAlloc = mAllocator.Allocate(sizeof(T));
			new(pAlloc) T(std::forward<Args3>(args)...);
		}
	};

	enum class EDelegateExecutionOrder : uint8
	{
		Undefined = 0,
		Sequential = 1,
		Reverse = 1,
	};

	//Delegate that can be bound to by MULTIPLE objects
	template <EDelegateExecutionOrder ExecutionOrder, typename... Args>
	class TMulticastDelegate : public FDelegateBase
	{
		GENERATED_BODY_MINIMAL(TMulticastDelegate, FDelegateBase)

	public:
		using DelegateT = TDelegate<void, Args...>;

	private:
		struct FDelegateHandlerPair
		{
			FDelegateHandle Handle;
			DelegateT Callback;

			FDelegateHandlerPair() : Handle(false)
			{
			}

			FDelegateHandlerPair(const FDelegateHandle& handle, const DelegateT& callback) : Handle(handle), Callback(callback)
			{
			}

			FDelegateHandlerPair(const FDelegateHandle& handle, DelegateT&& callback) : Handle(handle), Callback(std::move(callback))
			{
			}
		};

		template <typename T, typename... Args2>
		using ConstMemberFunction = typename _DelegatesInternal::TMemberFunction<true, T, void, Args..., Args2...>::Type;
		template <typename T, typename... Args2>
		using NonConstMemberFunction = typename _DelegatesInternal::TMemberFunction<false, T, void, Args..., Args2...>::Type;

	public:
		//Default constructor
		constexpr TMulticastDelegate()
			: mLocks(0)
		{
		}

		//Default destructor
		~TMulticastDelegate() noexcept = default;

		//Default copy constructor
		TMulticastDelegate(const TMulticastDelegate& other) = default;

		//Default copy assignment operator
		TMulticastDelegate& operator=(const TMulticastDelegate& other) = default;

		//Move constructor
		TMulticastDelegate(TMulticastDelegate&& other) noexcept
			: mEvents(std::move(other.mEvents)),
			  mLocks(std::move(other.mLocks))
		{
		}

		//Move assignment operator
		TMulticastDelegate& operator=(TMulticastDelegate&& other) noexcept
		{
			mEvents = std::move(other.mEvents);
			mLocks = std::move(other.mLocks);
			return *this;
		}

		void Clear() override
		{
			Super::Clear();

			RemoveAll();
		}

		FDelegateHandle Add(DelegateT&& handler) noexcept
		{
			//Favor an empty space over a possible array reallocation
			for (size_t i = 0; i < mEvents.size(); ++i)
			{
				if (mEvents[i].Handle.IsValid() == false)
				{
					mEvents[i] = FDelegateHandlerPair(FDelegateHandle(true), std::move(handler));
					return mEvents[i].Handle;
				}
			}
			mEvents.emplace_back(FDelegateHandle(true), std::move(handler));
			return mEvents.back().Handle;
		}

		//Bind a member function
		template <typename T, typename... Args2>
		FDelegateHandle AddRaw(T* pObject, NonConstMemberFunction<T, Args2...> pFunction, Args2&&... args)
		{
			return Add(DelegateT::CreateRaw(pObject, pFunction, std::forward<Args2>(args)...));
		}

		template <typename T, typename... Args2>
		FDelegateHandle AddRaw(T* pObject, ConstMemberFunction<T, Args2...> pFunction, Args2&&... args)
		{
			return Add(DelegateT::CreateRaw(pObject, pFunction, std::forward<Args2>(args)...));
		}

		//Bind a static/global function
		template <typename... Args2>
		FDelegateHandle AddStatic(void (*pFunction)(Args..., Args2...), Args2&&... args)
		{
			return Add(DelegateT::CreateStatic(pFunction, std::forward<Args2>(args)...));
		}

		//Bind a lambda
		template <typename LambdaType, typename... Args2>
		FDelegateHandle AddLambda(LambdaType&& lambda, Args2&&... args)
		{
			return Add(DelegateT::CreateLambda(std::forward<LambdaType>(lambda), std::forward<Args2>(args)...));
		}

		//Bind a member function with a shared_ptr object
		template <typename T, typename... Args2>
		FDelegateHandle AddSP(std::shared_ptr<T> pObject, NonConstMemberFunction<T, Args2...> pFunction, Args2&&... args)
		{
			return Add(DelegateT::CreateSP(pObject, pFunction, std::forward<Args2>(args)...));
		}

		template <typename T, typename... Args2>
		FDelegateHandle AddSP(std::shared_ptr<T> pObject, ConstMemberFunction<T, Args2...> pFunction, Args2&&... args)
		{
			return Add(DelegateT::CreateSP(pObject, pFunction, std::forward<Args2>(args)...));
		}

		//Removes all handles that are bound from a specific object
		//Ignored when pObject is null
		//Note: Only works on Raw and SP bindings
		void RemoveObject(void* objectPtr)
		{
			if (objectPtr != nullptr)
			{
				for (size_t i = 0; i < mEvents.size(); ++i)
				{
					if (mEvents[i].Callback.GetOwner() == objectPtr)
					{
						if (IsLocked())
						{
							mEvents[i].Callback.Clear();
						}
						else
						{
							std::swap(mEvents[i], mEvents[mEvents.size() - 1]);
							mEvents.pop_back();
						}
					}
				}
			}
		}

		//Remove a function from the event list by the handle
		bool Remove(FDelegateHandle& handle)
		{
			if (handle.IsValid())
			{
				for (size_t i = 0; i < mEvents.size(); ++i)
				{
					if (mEvents[i].Handle == handle)
					{
						if (IsLocked())
						{
							mEvents[i].Callback.Clear();
						}
						else
						{
							std::swap(mEvents[i], mEvents[mEvents.size() - 1]);
							mEvents.pop_back();
						}
						handle.Reset();
						return true;
					}
				}
			}
			return false;
		}

		[[nodiscard]] bool IsBoundTo(const FDelegateHandle& handle) const
		{
			if (handle.IsValid())
			{
				for (size_t i = 0; i < mEvents.size(); ++i)
				{
					if (mEvents[i].Handle == handle)
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
				for (FDelegateHandlerPair& handler : mEvents)
				{
					handler.Callback.Clear();
				}
			}
			else
			{
				mEvents.clear();
			}
		}

		void Compress(size_t maxSpace = 0)
		{
			if (IsLocked() == false)
			{
				size_t toDelete = 0;
				for (size_t i = 0; i < mEvents.size() - toDelete; ++i)
				{
					if (mEvents[i].Handle.IsValid() == false)
					{
						std::swap(mEvents[i], mEvents[toDelete]);
						++toDelete;
					}
				}
				if (toDelete > maxSpace)
				{
					mEvents.resize(mEvents.size() - toDelete);
				}
			}
		}

		//Execute all functions that are bound
		void Broadcast(Args... args)
		{
			Lock();
			if constexpr (ExecutionOrder == EDelegateExecutionOrder::Sequential)
			{
				for (const auto& [handle, callback] : std::ranges::reverse_view(mEvents))
				{
					if (handle.IsValid())
					{
						callback.Execute(args...);
					}
				}
			}
			else
			{
				for (const auto& [handle, callback] : mEvents)
				{
					if (handle.IsValid())
					{
						callback.Execute(args...);
					}
				}
			}
			Unlock();
		}

		[[nodiscard]] virtual size_t GetSize() const override
		{
			return mEvents.size();
		}

	private:
		void Lock()
		{
			++mLocks;
		}

		void Unlock()
		{
			TURBO_CHECK_MSG(mLocks > 0, "Unlock() should never be called more than Lock()!");
			--mLocks;
		}

		//Returns true is the delegate is currently broadcasting
		//If this is true, the order of the array should not be changed otherwise this causes undefined behaviour
		bool IsLocked() const
		{
			return mLocks > 0;
		}

		std::vector<FDelegateHandlerPair> mEvents;
		uint32 mLocks;
	};

} // Turbo
