#pragma once

namespace Turbo
{
	struct FRect2D final
	{
		glm::vec2 Position = {};
		glm::vec2 Size = {};
	};

	struct FRect2DInt final
	{
		glm::ivec2 Position = {};
		glm::ivec2 Size = {};
	};


	struct FViewport final
	{
		FRect2DInt Rect;
		float MinDepth = 0.f;
		float MaxDepth = 0.f;
	};
} // Turbo
