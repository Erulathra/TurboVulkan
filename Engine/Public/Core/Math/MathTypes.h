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

		static FRect2DInt FromSize(glm::ivec2 size)
		{
			return {glm::ivec2(0.f), size};
		}
	};


	struct FViewport final
	{
		FRect2DInt Rect;
		float MinDepth = 0.f;
		float MaxDepth = 0.f;
	};
} // Turbo
