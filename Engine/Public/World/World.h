#pragma once
#include "Core/DataStructures/Pool.h"

namespace Turbo
{
	class FCamera;

	class FWorld
	{
	public:
		THandle<FCamera> CreateCamera();

	private:
		entt::registry mRegistry;

		TPoolGrowable<FCamera> mCameras;
		THandle<FCamera> mCurrentCamera;
	};
} // Turbo