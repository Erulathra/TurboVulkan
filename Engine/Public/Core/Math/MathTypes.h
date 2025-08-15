#pragma once

namespace Turbo
{
	struct FRect2D
	{
		glm::vec2 Position{};
		glm::vec2 Size{};
	};

	struct FRect2DInt
	{
		glm::ivec2 Position{};
		glm::ivec2 Size{};
	};

	struct FViewport
	{
		FRect2DInt Rect{};
		float MinDepth = 0.f;
		float MaxDepth = 0.f;
	};
} // Turbo
