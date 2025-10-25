#include "World/World.h"

namespace Turbo
{
	THandle<FCamera> FWorld::CreateCamera()
	{
		entt::entity entity = mRegistry.create();

		return {};
	}
} // Turbo