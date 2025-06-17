#pragma once

namespace Turbo
{
	class IDeletable
	{
	public:
		virtual ~IDeletable() = default;

	public:
		virtual void Delete() = 0;
	};

	using FDeletionDelegate = std::function<void()>;

	class FDeletionQueue
	{

	public:
		void RequestDeletion(const std::shared_ptr<IDeletable>& objectToDelete) { mObjectsToDelete.push_back(objectToDelete); }
		void AddDelegate(const FDeletionDelegate& delegate) { mDelegatesToCall.push_back(delegate); }
		void Flush();

	private:
		std::vector<std::shared_ptr<IDeletable>> mObjectsToDelete;
		std::vector<FDeletionDelegate> mDelegatesToCall;
	};
} // Turbo
