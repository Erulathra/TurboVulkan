#pragma once

#include <typeindex>

#include "Core/RHI/RHICore.h"
#include "Core/Delegate.h"

namespace Turbo
{
	class FVulkanDevice;

	class IDestroyer
	{
	public:
		virtual ~IDestroyer() = default;

	public:
		virtual void Destroy(const FVulkanDevice* device) = 0;
	};

	class IDestroyQueue
	{
	public:
		virtual ~IDestroyQueue() = default;

	public:
		virtual void Flush(const FVulkanDevice* device) = 0;
	};

	template <typename DestroyerType> requires (std::is_base_of_v<IDestroyer, DestroyerType>)
	class TDestroyQueue : public IDestroyQueue
	{
	public:
		inline void RequestDestroy(const DestroyerType& destroyer) { mDestroyers.push_back(destroyer); }
		virtual void Flush(const FVulkanDevice* device) override
		{
			for (DestroyerType& destroyer : std::ranges::reverse_view(mDestroyers))
			{
				destroyer.Destroy(device);
			}

			mDestroyers.clear();
		}

	private:
		std::vector<DestroyerType> mDestroyers;
	};

	class FRHIDestroyQueue
	{
	public:
		DECLARE_MULTICAST_DELEGATE_REVERSE(FOnDestroy);

	public:
		template <typename DestroyerType> requires (std::is_base_of_v<IDestroyer, DestroyerType>)
		void RequestDestroy(const DestroyerType& destroyer)
		{
			using CastedQueueType= TDestroyQueue<DestroyerType>;

			const size_t typeID = typeid(destroyer).hash_code();

			IDestroyQueue* destroyQueue;

			if (const auto destroyQueueIt = mDestroyQueues.find(typeID); destroyQueueIt != mDestroyQueues.end())
			{
				destroyQueue = destroyQueueIt->second.get();
			}
			else
			{
				mDestroyQueues[typeID] = std::make_unique<CastedQueueType>();
				destroyQueue = mDestroyQueues[typeID].get();
			}

			CastedQueueType* castedQueue = static_cast<CastedQueueType*>(destroyQueue);
			castedQueue->RequestDestroy(destroyer);
		}

		void Flush(const FVulkanDevice* device);

	public:
		FOnDestroy& OnDestroy() { return mOnDestroy; }

		/* Delegates */
	private:
		std::unordered_map<size_t /* typeID */, std::unique_ptr<IDestroyQueue>> mDestroyQueues;

		FOnDestroy mOnDestroy;
	};
} // Turbo
