#pragma once

namespace Turbo
{
	template<typename ScalarType>
		requires std::is_floating_point_v<ScalarType>
	struct TRotator
	{
	private:
		using ScalarType3 = glm::vec<3, ScalarType>;
		using QuatType = glm::qua<ScalarType>;

	public:
		ScalarType mPitch;
		ScalarType mRoll;
		ScalarType mYaw;

	public:
		TRotator(ScalarType x)
			:TRotator(x, x, x)
		{

		}

		TRotator(ScalarType pitch, ScalarType yaw, ScalarType roll)
			: mPitch(pitch)
			, mRoll(roll)
			, mYaw(yaw)
		{ }

		explicit TRotator(const ScalarType3& eulerAngles)
			: TRotator(eulerAngles.x, eulerAngles.y, eulerAngles.z)
		{
		}

		[[nodiscard]]
		static TRotator FromQuat(const QuatType& quat)
		{
			const ScalarType3 glmEuler = glm::eulerAngles(quat);
			return TRotator(glmEuler);
		}

		[[nodiscard]]
		ScalarType3 ToEuler() const
		{
			return ScalarType3(mPitch, mYaw, mRoll);
		}

		[[nodiscard]]
		QuatType ToQuat() const
		{
			return glm::quat(-ScalarType3(mPitch, mYaw, mRoll));
		}

		[[nodiscard]]
		TRotator Normalize() const
		{
			return TRotator(
				FMath::NormalizeAngle(mPitch),
				FMath::NormalizeAngle(mYaw),
				FMath::NormalizeAngle(mRoll)
			);
		}

		[[nodiscard]]
		ScalarType3 Forward() const
		{
			return glm::normalize(ToQuat() * EFloat3::Forward);
		}

		[[nodiscard]]
		ScalarType3 Up() const
		{
			return glm::normalize(ToQuat() * EFloat3::Up);
		}

		[[nodiscard]]
		ScalarType3 Right() const
		{
			return glm::normalize(ToQuat() * EFloat3::Right);
		}

	public:
		TRotator& operator=(TRotator const& other)
		{
			std::tie(mPitch, mRoll, mYaw) = std::tie(other.mPitch, other.mRoll, other.mYaw);
			return *this;
		}

		TRotator operator-() const
		{
			return TRotator(
				-mPitch,
				-mYaw,
				-mRoll
			);
		}

		TRotator operator-(TRotator const& other) const
		{
			return TRotator(
				mPitch - other.mPitch,
				mYaw - other.mYaw,
				mRoll - other.mRoll
			);
		}

		TRotator operator+(TRotator const& other) const
		{
			return TRotator(
				mPitch + other.mPitch,
				mYaw + other.mYaw,
				mRoll + other.mRoll
			);
		}

		TRotator operator*(const ScalarType& scalar) const
		{
			return TRotator(
				mPitch * scalar,
				mYaw * scalar,
				mRoll * scalar
			);
		}

		void operator+=(TRotator const& other)
		{
			*this = *this + other;
		}

		void operator-=(TRotator const& other)
		{
			*this = *this - other;
		}

		void operator*=(ScalarType const& scalar)
		{
			*this = *this * scalar;
		}

		bool operator==(const TRotator& other) const
		{
			return mPitch == other.mPitch
				&& mRoll == other.mRoll
				&& mYaw == other.mYaw;
		}

		bool operator!=(const TRotator& other) const
		{
			return !(*this == other);
		}

	};

	using FRotator = TRotator<float>;
}
