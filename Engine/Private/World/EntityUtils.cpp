#include "World/EntityUtils.h"

#include "World/SceneGraph.h"

namespace Turbo
{
	std::string EntityUtils::GetEntityLabel(const entt::registry& registry, entt::entity entity)
	{
		std::string_view name = "None";
		if (const FEntityLabel* entityLabel = registry.try_get<FEntityLabel>(entity))
		{
			name = entityLabel->mName.ToString();
		}

		return std::string(name);
	}
} // namespace Turbo