#include "Core/CoreTimer.h"

#include "Core/Engine.h"

namespace Turbo {
	void FCoreTimer::Init()
	{
		mEngineStartTime = std::chrono::steady_clock::now();
	}

	void FCoreTimer::Destroy()
	{
		// do nothing
	}

	FCoreTimer* FCoreTimer::Get()
	{
		if (entt::locator<FCoreTimer>::has_value())
		{
			return &entt::locator<FCoreTimer>::value();
		}

		return nullptr;
	}

	void FCoreTimer::Tick()
	{
		if (mTickIndex == 0)
		{
			mDeltaTime = 0;
			mTimeFromEngineStart = 0;

			mTickStartTime = std::chrono::steady_clock::now();
			++mTickIndex;

			return;
		}

		const FChronoTimePoint newTickStartTime = std::chrono::steady_clock::now();

		mDeltaTime = std::chrono::duration<double>(newTickStartTime - mTickStartTime).count();
		mTimeFromEngineStart = std::chrono::duration<double>(newTickStartTime - mEngineStartTime).count();

		mTickStartTime = newTickStartTime;
		++mTickIndex;
	}
} // Turbo