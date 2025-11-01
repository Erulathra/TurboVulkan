#include "World/World.h"

using namespace entt::literals;

namespace Turbo
{
	void MarkDirty_Impl(entt::registry& registry, entt::entity entity)
	{
		registry.emplace<FWorldTransformDirty>(entity);
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
		mRegistry.group<FRelationship, FWorldTransformDirty, FTransform, FWorldTransform>();

		mRegistry.on_construct<FTransform>().connect<MarkDirty_Impl>();
		mRegistry.on_construct<FTransform>().connect<AddWorldTransform>();
		mRegistry.on_update<FTransform>().connect<MarkDirty_Impl>();
	}

	void FWorld::UpdateWorldTransforms()
	{
		TRACE_ZONE_SCOPED();

		PropagateDirty();
		ReCalculateDirtyTransforms();
	}

	void FWorld::PropagateDirty()
	{
		TRACE_ZONE_SCOPED();

		std::vector<entt::entity> entitiesToProcess;
		entt::dense_set<entt::entity> processedEntities;
		int32 entitiesToProcessBack = 0;

		{
			auto dirtyTransformsGroup = mRegistry.view<FRelationship, FWorldTransformDirty>();
			for (entt::entity entity : dirtyTransformsGroup)
			{
				// entitiesToProcess.push_back(entity);
				processedEntities.insert(entity);
			}
		}

		// Iterate over children using breath first search
		while (entitiesToProcessBack < entitiesToProcess.size())
		{
			entt::entity parent = entitiesToProcess[entitiesToProcessBack];
			entitiesToProcessBack++;

			EachChild(parent, [&](entt::entity child)
			{
				if (processedEntities.insert(child).second)
				{
					entitiesToProcess.push_back(child);
				}
			});
		}

		TRACE_PLOT_VALUE("Dirty transforms", static_cast<int64>(processedEntities.size()));

		for (entt::entity entity : entitiesToProcess)
		{
			mRegistry.emplace<FWorldTransformDirty>(entity);
		}
	}

	void FWorld::ReCalculateDirtyTransforms()
	{
		TRACE_ZONE_SCOPED();


		auto dirtyTransformGroup = mRegistry.group<FRelationship, FWorldTransformDirty, FTransform, FWorldTransform>();
		{
			TRACE_ZONE_SCOPED_N("Sort dirty transforms")

			dirtyTransformGroup.sort(
				[&](entt::entity lhs, entt::entity rhs)
				{
					const FRelationship& lhsRel = mRegistry.get<FRelationship>(lhs);
					const FRelationship& rhsRel = mRegistry.get<FRelationship>(rhs);

					return rhsRel.mParent == lhs
						|| lhsRel.mNext == rhs
						|| (!(lhsRel.mParent == rhs || rhsRel.mNext == lhs) && (lhsRel.mParent < rhsRel.mParent || (lhsRel.mParent == rhsRel.mParent && &lhsRel < &rhsRel)));
				});
		}

		{
			TRACE_ZONE_SCOPED_N("Recalculate dirty transforms")

			for (const entt::entity dirtyEntity : dirtyTransformGroup)
			{
				const FRelationship& relationship = dirtyTransformGroup.get<FRelationship>(dirtyEntity);
				const FTransform& local = dirtyTransformGroup.get<FTransform>(dirtyEntity);
				FWorldTransform& world = dirtyTransformGroup.get<FWorldTransform>(dirtyEntity);

				const glm::mat4 localMatrix = FMath::CreateTransform(local.mPosition, local.mRotation, local.mScale);

				if (relationship.mParent == entt::null)
				{
					world.mTransform = localMatrix;
				}
				else
				{
					const FWorldTransform& parentWorld = dirtyTransformGroup.get<FWorldTransform>(dirtyEntity);
					world.mTransform = localMatrix * parentWorld.mTransform;
				}
			}
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
