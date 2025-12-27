#pragma once

#include "Core/DataStructures/GenPoolGrowable.h"
#include "Core/DataStructures/Handle.h"

DECLARE_LOG_CATEGORY(LogAssetManager, Info, Info)

namespace Turbo
{
	struct FMesh;

	template<typename AssetType>
	struct TAssetHandle
	{
		TAssetHandle()
			: mHandle()
		{

		}

		explicit TAssetHandle(THandle<AssetType> handle);

		TAssetHandle(const TAssetHandle& other) noexcept;
		TAssetHandle(TAssetHandle&& other) noexcept;
		TAssetHandle& operator=(const TAssetHandle& other) noexcept;

		~TAssetHandle();

		operator THandle<AssetType>()
		{
			return mHandle;
		}

		explicit constexpr operator bool() const { return !!mHandle; }
		constexpr bool operator!() const { return !mHandle; }

		constexpr bool operator==(const TAssetHandle& rhs) const
		{
			return this->mHandle == rhs.mHandle;
		}

		constexpr bool operator!=(const TAssetHandle& rhs) const
		{
			return !(*this == rhs);
		}

	public:
		THandle<AssetType> mHandle;
	};

	template<typename AssetType>
	struct FAssetManager
	{
	// Constants and type aliases
		using RefCounterType = uint16;
		static constexpr FHandle::IndexType kInitialPoolSize = 1024;

		using AssetLookUpType = entt::dense_map<FName, THandle<AssetType>>;
		using RefCountersVectorType = std::vector<RefCounterType>;
		using AssetPoolType = TGenPoolGrowable<AssetType>;

	// Manager interface
		[[nodiscard]] static TAssetHandle<AssetType> GetAsset(FName assetPath)
		{
			AssetLookUpType& assetLookUp = GetAssetLookUp();
			AssetPoolType& assetPool = GetPool();
			RefCountersVectorType& refCounters = GetRefCounters();

			if (auto foundAssetIt = assetLookUp.find(assetPath);
				foundAssetIt != assetLookUp.end())
			{
				return TAssetHandle<AssetType>(foundAssetIt->second);
			}

			TURBO_LOG(LogAssetManager, Info, "Trying to load {} asset.", assetPath);

			THandle<AssetType> newAssetHandle = assetPool.Acquire();
			AssetType* newAsset = assetPool.Access(newAssetHandle);

			// try to load asset
			if (TryLoadAsset(assetPath, newAssetHandle, *newAsset) == false)
			{
				TURBO_LOG(LogAssetManager, Info, "Loading {} failed.", assetPath)

				// return null handle on error
				assetPool.Release(newAssetHandle);
				return TAssetHandle<AssetType>();
			}

			assetLookUp[assetPath] = newAssetHandle;

			// resize refcounter vector if required and reset asset's refcounter
			const FHandle::IndexType assetPoolSize = assetPool.Size();
			if (assetPoolSize > refCounters.size())
			{
				FHandle::IndexType oldSize = refCounters.size();
				refCounters.resize(assetPoolSize);

				std::memset(&refCounters[oldSize], 0, assetPoolSize - oldSize);
			}

			return TAssetHandle<AssetType>(newAssetHandle);
		}

		static AssetType* AccessAsset(const TAssetHandle<AssetType>& handle)
		{
			return GetPool().Access(handle);
		}

		static AssetType* AccessAsset(const THandle<AssetType>& handle)
		{
			return GetPool().Access(handle);
		}

		static void AccessReference(const TAssetHandle<AssetType>& handle)
		{
			AssetPoolType& assetPool = GetPool();
			RefCountersVectorType& refCounters = GetRefCounters();

			const AssetType* asset = assetPool.Access(handle.mHandle);
			TURBO_CHECK(asset != nullptr)

			++refCounters[handle.mHandle.GetIndex()];
		}

		static void ReleaseReference(const TAssetHandle<AssetType>& handle)
		{
			AssetLookUpType& assetLookUp = GetAssetLookUp();
			AssetPoolType& assetPool = GetPool();
			RefCountersVectorType& refCounters = GetRefCounters();

			const AssetType* asset = assetPool.Access(handle.mHandle);
			TURBO_CHECK(asset != nullptr)

			RefCounterType& refCounter = refCounters[handle.mHandle.GetIndex()];
			--refCounter;

			if (refCounter > 0)
			{
				return;
			}

			TURBO_LOG(LogTemp, Info, "Unloading {}", asset->mName);

			assetLookUp.erase(asset->mName);

			UnloadAsset(handle.mHandle, *asset);
			assetPool.Release(handle.mHandle);
		}

	private:
	// Helper Methods
		static bool TryLoadAsset(FName assetPath, THandle<AssetType> assetHandle, AssetType& outLoadedAsset)
		{
			// ReSharper disable once CppStaticAssertFailure
			static_assert(false, "You need to create template specialization for this type");
			return false;
		}

		static void UnloadAsset(THandle<AssetType> assetHandle, const AssetType& unloadedAsset)
		{
			// ReSharper disable once CppStaticAssertFailure
			static_assert(false, "You need to create template specialization for this type");
		}

		[[nodiscard]] static AssetPoolType& GetPool()
		{
			static TGenPoolGrowable<AssetType> assetPool(kInitialPoolSize);
			return assetPool;
		}

		[[nodiscard]] static RefCountersVectorType& GetRefCounters()
		{
			static std::vector<RefCounterType> referenceCounters(0);
			return referenceCounters;
		}

		[[nodiscard]] static AssetLookUpType& GetAssetLookUp()
		{
			static entt::dense_map<FName, THandle<AssetType>> assetLookUp = {};
			return assetLookUp;
		}

	};

	template <typename AssetType>
	TAssetHandle<AssetType>::TAssetHandle(THandle<AssetType> handle)
		:mHandle(handle)
	{
		if (mHandle)
		{
			FAssetManager<AssetType>::AccessReference(*this);
		}
	}

	template <typename AssetType>
	TAssetHandle<AssetType>::TAssetHandle(const TAssetHandle& other) noexcept
	{
		if (*this != other)
		{
			if (other)
			{
				FAssetManager<AssetType>::ReleaseReference(*this);
				FAssetManager<AssetType>::AccessReference(other);
			}

			this->mHandle = other.mHandle;
		}

		return *this;
	}

	template <typename AssetType>
	TAssetHandle<AssetType>::TAssetHandle(TAssetHandle&& other) noexcept
	{
		if (*this)
		{
			FAssetManager<AssetType>::ReleaseReference(*this);
		}

		if (other)
		{
			this->mHandle = other;
			other.mHandle = {};
		}
	}

	template <typename AssetType>
	TAssetHandle<AssetType>& TAssetHandle<AssetType>::operator=(const TAssetHandle& other) noexcept
	{
		if (*this != other)
		{
			if (*this)
			{
				FAssetManager<AssetType>::ReleaseReference(*this);
			}

			if (other)
			{
				FAssetManager<AssetType>::AccessReference(other);
			}

			this->mHandle = other.mHandle;
		}

		return *this;
	}

	template <typename AssetType>
	TAssetHandle<AssetType>::~TAssetHandle()
	{
		if (mHandle)
		{
			FAssetManager<AssetType>::ReleaseReference(*this);
		}
	}
}
