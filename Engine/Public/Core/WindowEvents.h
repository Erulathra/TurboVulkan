#pragma once

#include "Layers/Event.h"

namespace Turbo
{
	class FWindow;

	struct FCloseWindowEvent : FEventBase
	{
		EVENT_BODY(FCloseWindowEvent)
	};

	struct FResizeWindowEvent : FEventBase
	{
		EVENT_BODY(FResizeWindowEvent)

		glm::uint2 mNewWindowSize;
	};
}

