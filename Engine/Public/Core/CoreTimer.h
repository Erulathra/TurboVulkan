#pragma once

namespace Turbo
{
	using FChronoTimePoint = std::chrono::time_point<std::chrono::steady_clock>;

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
		[[nodiscard]] static uint64 TickIndex() { return Get()->GetTickIndex(); }

		DELETE_COPY(FCoreTimer)

	protected:
		void Tick();

	private:
		[[nodiscard]] double GetDeltaTime() const { return mDeltaTime; }
		[[nodiscard]] double GetTimeFromEngineStart() const { return mTimeFromEngineStart; };
		[[nodiscard]] uint64 GetTickIndex() const { return mTickIndex; };

	private:
		FChronoTimePoint mEngineStartTime {};
		FChronoTimePoint mTickStartTime {};

		double mDeltaTime = -1.;
		double mTimeFromEngineStart = -1.;
		uint64 mTickIndex = 0;

	public:
		friend class FEngine;
	};
} // Turbo
