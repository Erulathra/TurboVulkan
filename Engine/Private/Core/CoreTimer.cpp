#include "Core/CoreTimer.h"

#include "Core/Engine.h"

namespace Turbo {
	void FCoreTimer::Init()
	{
		mEngineStartTime = std::chrono::high_resolution_clock::now();
	}

	void FCoreTimer::Destroy()
	{
		// do nothing
	}

	FCoreTimer* FCoreTimer::Get()
	{
		return gEngine->GetTimer();
	}

	void FCoreTimer::Tick()
	{
		if (bFirstTick)
		{
			mDeltaTime = 0;
			mTimeFromEngineStart = 0;

			mTickStartTime = std::chrono::high_resolution_clock::now();

			bFirstTick = false;

			return;
		}

		std::chrono::time_point<std::chrono::system_clock> newTickStartTime = std::chrono::high_resolution_clock::now();

		mDeltaTime = std::chrono::duration<double>(newTickStartTime - mTickStartTime).count();
		mTimeFromEngineStart = std::chrono::duration<double>(newTickStartTime - mEngineStartTime).count();

		mTickStartTime = newTickStartTime;
	}
} // Turbo