#pragma once

namespace Turbo
{
	class FCoreTimer
	{
	private:
		FCoreTimer() = default;

	public:
		void Init();
		void Destroy();

		static FCoreTimer* Get();
		[[nodiscard]] static double DeltaTime() { return Get()->GetDeltaTime(); }
		[[nodiscard]] static double TimeFromEngineStart() { return Get()->GetTimeFromEngineStart(); }

	protected:
		void Tick();

	private:
		[[nodiscard]] double GetDeltaTime() const { return mDeltaTime; }
		[[nodiscard]] double GetTimeFromEngineStart() const { return mTimeFromEngineStart; };

	private:
		std::chrono::time_point<std::chrono::system_clock> mEngineStartTime {};
		std::chrono::time_point<std::chrono::system_clock> mTickStartTime {};

		bool bFirstTick = true;

		double mDeltaTime = -1.;
		double mTimeFromEngineStart = -1.;

	public:
		friend class FEngine;
	};
} // Turbo
