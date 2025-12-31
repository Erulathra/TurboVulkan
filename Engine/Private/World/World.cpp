#include "World/World.h"

using namespace entt::literals;

namespace Turbo
{
	void MarkDirty_Impl(entt::registry& registry, entt::entity entity)
	{
		registry.emplace_or_replace<FWorldTransformDirty>(entity);
	}

	void AddWorldTransform(entt::registry& registry, entt::entity entity)
	{
		registry.emplace<FWorldTransform>(entity);
	}

	void FWorld::MarkDirty(entt::entity entity)
	{
		MarkDirty_Impl(mRegistry, entity);
	}

	void FWorld::InitSceneGraph()
	{
		mRegistry.on_construct<FTransform>().connect<MarkDirty_Impl>();
		mRegistry.on_construct<FTransform>().connect<AddWorldTransform>();
		mRegistry.on_update<FTransform>().connect<MarkDirty_Impl>();
	}

	void FWorld::UpdateWorldTransforms()
	{
		TRACE_ZONE_SCOPED();

		ReCalculateDirtyTransforms();
	}

	void FWorld::ReCalculateDirtyTransforms()
	{
		TRACE_ZONE_SCOPED();

		{
			TRACE_ZONE_SCOPED_N("Recalculate dirty transforms (DFS)")
			std::vector<entt::entity> entitiesToProcess;
			entt::dense_set<entt::entity> processedEntities;
			int32 entitiesToProcessBack = 0;

			{
				auto dirtyTransformsView = mRegistry.view<FRelationship, FWorldTransformDirty>();
				for (entt::entity entity : dirtyTransformsView)
				{
					entitiesToProcess.push_back(entity);
					processedEntities.insert(entity);
				}
			}

			auto transformView = mRegistry.view<FRelationship, FTransform, FWorldTransform>();

			// Iterate over children using breath first search
			while (entitiesToProcessBack < entitiesToProcess.size())
			{
				entt::entity dirtyEntity = entitiesToProcess[entitiesToProcessBack];
				entitiesToProcessBack++;

				const FRelationship& relationship = transformView.get<FRelationship>(dirtyEntity);
				const FTransform& local = transformView.get<FTransform>(dirtyEntity);
				FWorldTransform& world = transformView.get<FWorldTransform>(dirtyEntity);

				const glm::mat4 localMatrix = FMath::CreateTransform(local.mPosition, local.mRotation, local.mScale);

				if (relationship.mParent == entt::null)
				{
					world.mTransform = localMatrix;
				}
				else
				{
					const FWorldTransform& parentWorld = transformView.get<FWorldTransform>(relationship.mParent);
					world.mTransform = parentWorld.mTransform * localMatrix;
				}

				EachChild(dirtyEntity, [&](entt::entity child)
				{
					if (processedEntities.insert(child).second)
					{
						entitiesToProcess.push_back(child);
					}
				});
			}

			TRACE_PLOT("Dirty transforms", static_cast<int64>(processedEntities.size()));
		}

		{
			TRACE_ZONE_SCOPED_N("Clear dirty flags")

			const auto dirtyView = mRegistry.view<FWorldTransformDirty>();
			mRegistry.remove<FWorldTransformDirty>(dirtyView.begin(), dirtyView.end());
		}
	}

	entt::entity FWorld::GetParent(entt::entity entity)
	{
		const FRelationship& relationship = mRegistry.get<FRelationship>(entity);
		return relationship.mParent;
	}

	void FWorld::AddChild(entt::entity parent, entt::entity child)
	{
		FRelationship& childRel = mRegistry.get<FRelationship>(child);
		if (childRel.mParent != entt::null)
		{
			Unparent(child);
		}

		FRelationship& parentRel = mRegistry.get<FRelationship>(parent);

		childRel.mParent = parent;
		childRel.mNext = parentRel.mFirstChild;

		parentRel.mFirstChild = child;
		parentRel.mNumChildren++;
	}

	void FWorld::Unparent(entt::entity child)
	{
		FRelationship& childRel = mRegistry.get<FRelationship>(child);
		if (childRel.mParent == entt::null)
		{
			return;
		}

		FRelationship& leftRel = mRegistry.get<FRelationship>(childRel.mPrevious);
		FRelationship& rightRel = mRegistry.get<FRelationship>(childRel.mNext);

		leftRel.mNext = childRel.mNext;
		rightRel.mPrevious = childRel.mPrevious;

		FRelationship& parentRel = mRegistry.get<FRelationship>(childRel.mParent);
		parentRel.mNumChildren--;

		if (parentRel.mFirstChild == child)
		{
			parentRel.mFirstChild = childRel.mNext;
		}

		childRel.mParent = entt::null;
	}


} // Turbo
