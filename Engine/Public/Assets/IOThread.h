#pragma once

#include "TaskScheduler.h"

namespace Turbo
{
	struct FIOThread : enki::IPinnedTask
	{
		virtual void Execute() override;
	};

	struct FIOTask : enki::IPinnedTask
	{
		virtual void Execute() override;
	};

	struct FAsyncLoadingManager
	{
	public:
		void Update();
	};
}
