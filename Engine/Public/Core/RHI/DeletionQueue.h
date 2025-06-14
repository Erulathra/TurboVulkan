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

	class FDeletionQueue
	{
	public:
		void RequestDeletion(const std::shared_ptr<IDeletable>& objectToDelete) { mObjectsToDelete.push_back(objectToDelete); }
		void Flush();

	private:
		std::vector<std::shared_ptr<IDeletable>> mObjectsToDelete;
	};
} // Turbo
