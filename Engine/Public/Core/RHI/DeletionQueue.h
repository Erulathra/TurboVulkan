#pragma once
#include "Core/Delegate.h"

namespace Turbo
{
	class IDeletable
	{
	public:
		virtual ~IDeletable() = default;

	public:
		virtual void Delete() = 0;
	};


	class FDeletionQueue
	{
	public:
		DECLARE_MULTICAST_DELEGATE_REVERSE(FOnDeletion);

	public:
		void RequestDeletion(const std::shared_ptr<IDeletable>& objectToDelete) { mObjectsToDelete.push_back(objectToDelete); }
		void Flush();

	public:
		FOnDeletion& OnDeletion() { return mOnDeletion; }

	private:
		std::vector<std::shared_ptr<IDeletable>> mObjectsToDelete;
		FOnDeletion mOnDeletion;
	};
} // Turbo
