#include "World/SceneGraph.h"

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

	void FSceneGraph::InitSceneGraph(entt::registry& registry)
	{
		registry.on_construct<FTransform>().connect<MarkDirty_Impl>();
		registry.on_construct<FTransform>().connect<AddWorldTransform>();
		registry.on_update<FTransform>().connect<MarkDirty_Impl>();
	}

	void FSceneGraph::UpdateWorldTransforms(entt::registry& registry)
	{
		TRACE_ZONE_SCOPED();

		{
			TRACE_ZONE_SCOPED_N("Recalculate dirty transforms (DFS)")
			std::vector<entt::entity> entitiesToProcess;
			entt::dense_set<entt::entity> processedEntities;
			int32 entitiesToProcessBack = 0;

			{
				auto dirtyTransformsView = registry.view<FRelationship, FWorldTransformDirty>();
				for (entt::entity entity : dirtyTransformsView)
				{
					entitiesToProcess.push_back(entity);
					processedEntities.insert(entity);
				}
			}

			auto transformView = registry.view<FRelationship, FTransform, FWorldTransform>();

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

				EachChild(registry, dirtyEntity, [&](entt::entity child)
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

			const auto dirtyView = registry.view<FWorldTransformDirty>();
			registry.remove<FWorldTransformDirty>(dirtyView.begin(), dirtyView.end());
		}
	}

	void FSceneGraph::AddChild(entt::registry& registry, entt::entity parent, entt::entity child)
	{
		FRelationship& childRel = registry.get<FRelationship>(child);
		if (childRel.mParent != entt::null)
		{
			Unparent(registry, child);
		}

		FRelationship& parentRel = registry.get<FRelationship>(parent);

		childRel.mParent = parent;
		childRel.mNext = parentRel.mFirstChild;

		parentRel.mFirstChild = child;
		parentRel.mNumChildren++;
	}

	void FSceneGraph::Unparent(entt::registry& registry, entt::entity child)
	{
		FRelationship& childRel = registry.get<FRelationship>(child);
		if (childRel.mParent == entt::null)
		{
			return;
		}

		FRelationship& leftRel = registry.get<FRelationship>(childRel.mPrevious);
		FRelationship& rightRel = registry.get<FRelationship>(childRel.mNext);

		leftRel.mNext = childRel.mNext;
		rightRel.mPrevious = childRel.mPrevious;

		FRelationship& parentRel = registry.get<FRelationship>(childRel.mParent);
		parentRel.mNumChildren--;

		if (parentRel.mFirstChild == child)
		{
			parentRel.mFirstChild = childRel.mNext;
		}

		childRel.mParent = entt::null;
	}

	void FSceneGraph::MarkDirty(entt::registry& registry, entt::entity entity)
	{
		MarkDirty_Impl(registry, entity);
	}

} // Turbo