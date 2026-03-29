#include "World/CommonEntities.h"

#include "World/MeshComponent.h"
#include "World/SceneGraph.h"

namespace Turbo
{
	entt::entity CommonEntities::SpawnStaticMeshEntity(entt::registry& registry)
	{
		const entt::entity entity = registry.create();
		registry.emplace<FMeshComponent>(entity);
		registry.emplace<FTransform>(entity);
		registry.emplace<FRelationship>(entity);

		return entity;
	}
} // Turbo