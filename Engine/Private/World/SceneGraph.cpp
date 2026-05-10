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
		registry.emplace<FWorldRoot>(entity);
	}

	void SceneGraph::InitSceneGraph(entt::registry& registry)
	{
		registry.on_construct<FTransform>().connect<MarkDirty_Impl>();
		registry.on_construct<FTransform>().connect<AddWorldTransform>();
		registry.on_update<FTransform>().connect<MarkDirty_Impl>();
	}

	void SceneGraph::UpdateWorldTransforms(entt::registry& registry)
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

			// Iterate over children using breath-first search
			while (entitiesToProcessBack < entitiesToProcess.size())
			{
				entt::entity dirtyEntity = entitiesToProcess[entitiesToProcessBack];
				entitiesToProcessBack++;

				const FRelationship& relationship = transformView.get<FRelationship>(dirtyEntity);

				if (const FTransform* local = registry.try_get<FTransform>(dirtyEntity))
				{
					FWorldTransform& world = transformView.get<FWorldTransform>(dirtyEntity);

					const glm::mat4 localMatrix = FMath::MatrixFromTransform(*local);

					if (relationship.mParent == entt::null)
					{
						world.mTransform = localMatrix;
					}
					else
					{
						const FWorldTransform& parentWorld = transformView.get<FWorldTransform>(relationship.mParent);
						world.mTransform = parentWorld.mTransform * localMatrix;
					}
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
	}

	void SceneGraph::ClearDirtyFlags(entt::registry& registry)
	{
		TRACE_ZONE_SCOPED_N("Clear dirty flags")

		const auto dirtyView = registry.view<FWorldTransformDirty>();
		registry.remove<FWorldTransformDirty>(dirtyView.begin(), dirtyView.end());
	}

	void SceneGraph::AddChild(entt::registry& registry, entt::entity parent, entt::entity child)
	{
		FRelationship& childRel = registry.get<FRelationship>(child);
		if (childRel.mParent != entt::null)
		{
			Unparent(registry, child);
		}

		FRelationship& parentRel = registry.get<FRelationship>(parent);

		childRel.mParent = parent;
		registry.remove<FWorldRoot>(child);
		childRel.mNext = parentRel.mFirstChild;

		parentRel.mFirstChild = child;
		parentRel.mNumChildren++;
	}

	void SceneGraph::Unparent(entt::registry& registry, entt::entity child)
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
		registry.emplace<FWorldRoot>(child);
	}

	void SceneGraph::MarkDirty(entt::registry& registry, entt::entity entity)
	{
		MarkDirty_Impl(registry, entity);
	}

	glm::float4x4 SceneGraph::GetParentWorldTransform(entt::registry& registry, entt::entity entity)
	{
		glm::float4x4 result = glm::float4x4(1.f);

		if (const FRelationship* relationship = registry.try_get<FRelationship>(entity))
		{
			if (const FWorldTransform* worldTransform = registry.try_get<FWorldTransform>(relationship->mParent))
			{
				result = worldTransform->mTransform;
			}
		}

		return result;
	}
} // Turbo