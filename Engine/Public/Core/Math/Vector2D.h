#pragma once

#include <glm/glm.hpp>
#include "glm/gtc/type_ptr.inl"

#include "CommonTypeDefs.h"

namespace Turbo
{
	template <typename T = float> requires (std::is_arithmetic_v<T>)
	class TVector2D : glm::tvec2<T>
	{
	private:
		using Super = glm::tvec2<T>;
	public:
		using Super::x;
		using Super::y;

	public:
		TVector2D() : Super(0, 0) {};
		TVector2D(T Value) : Super(Value, Value) {};
		TVector2D(T X, T Y) : Super(X, Y) {};

		template<typename InnerT>
		explicit TVector2D(TVector2D<InnerT> Value) : Super(Value.x, Value.y) {};

	public:
		T* Data() { return glm::value_ptr(this); }

		static T Dot(const TVector2D& Lhs, const TVector2D& RHS) { return glm::dot(Lhs, RHS); }
		static T Cross(const TVector2D& Lhs, const TVector2D& RHS) { return glm::cross(Lhs, RHS); }
	};

	using FVector2 = TVector2D<float>;
	using FIntVector2 = TVector2D<int32>;
	using FUIntVector2 = TVector2D<uint32>;
} // Turbo
