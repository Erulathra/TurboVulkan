#pragma once

#include "Assets/AssetManager.h"
#include "World/SceneGraph.h"
#include "Core/DataStructures/Handle.h"

namespace Turbo
{
	class FCamera;
	struct FMesh;
	struct FTexture;

	struct FRuntimeLevel
	{
		std::vector<THandle<FMesh>> mLoadedMeshes;
		std::vector<THandle<FTexture>> mLoadedTextures;
	};

	class FWorld
	{
	public:
		void OpenLevel(FName path);
		void UnloadLevel();

		/** Scene Graph */
	public:
		void InitSceneGraph();
		void UpdateWorldTransforms();

		entt::entity GetParent(entt::entity entity);
		void AddChild(entt::entity parent, entt::entity child);
		void Unparent(entt::entity child);

		void MarkDirty(entt::entity entity);

		template<typename Func>
		void EachChild(entt::entity entity, Func func)
		{
			const FRelationship& componentRel = mRegistry.get<FRelationship>(entity);
			entt::entity currentEntt = componentRel.mFirstChild;

			for (uint32 ChildId = 0; ChildId < componentRel.mNumChildren; ++ChildId)
			{
				std::invoke(func, currentEntt);

				const FRelationship& currentRel = mRegistry.get<FRelationship>(currentEntt);
				currentEntt = currentRel.mNext;
			}
		}
		/** Scene Graph end */

	private:
		void ReCalculateDirtyTransforms();

	public:
		entt::registry mRegistry;

		FRuntimeLevel mRuntimeLevel;
	};
} // Turbo
