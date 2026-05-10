#pragma once

namespace glm
{
	typedef uint32						uint1;			//!< \brief unsigned integer vector with 1 component. (From GLM_GTX_compatibility extension)
	typedef vec<2, uint32, highp>		uint2;			//!< \brief unsigned integer vector with 2 components. (From GLM_GTX_compatibility extension)
	typedef vec<3, uint32, highp>		uint3;			//!< \brief unsigned integer vector with 3 components. (From GLM_GTX_compatibility extension)
	typedef vec<4, uint32, highp>		uint4;			//!< \brief unsigned integer vector with 4 components. (From GLM_GTX_compatibility extension)
}

namespace Turbo
{
	struct FTransform
	{
		glm::float3 mPosition = {};
		glm::quat mRotation = glm::quat(glm::float3(0.f));
		glm::float3 mScale = glm::float3{1.f};
	};

	struct FRect2D final
	{
		glm::float2 Position = {};
		glm::float2 Size = {};
	};

	struct FRect2DInt final
	{
		glm::uint2 Position = {};
		glm::uint2 Size = {};

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

		static FViewport FromSize(glm::ivec2 size)
		{
			FViewport result;
			result.Rect = FRect2DInt::FromSize(size);
			result.MinDepth = 0.f;
			result.MaxDepth = 1.f;

			return result;
		}
	};

	struct FPlane final
	{
		glm::float3 mNormal = {};
		float mDistance = 0.f;

		FPlane() = default;

		FPlane(glm::float3 normal, float distance);
		FPlane(glm::float3 normal, glm::float3 point);
	};

	struct FFrustum final
	{
		// 0: near, 1: far, 2: left, 3: right, 4: top, 5: bottom
		FPlane mPlanes[6];

		FPlane& GetNear() { return mPlanes[0]; }
		FPlane& GetFar() { return mPlanes[1]; }
		FPlane& GetLeft() { return mPlanes[2]; }
		FPlane& GetRight() { return mPlanes[3]; }
		FPlane& GetTop() { return mPlanes[4]; }
		FPlane& GetBottom() { return mPlanes[5]; }
	};

} // Turbo
