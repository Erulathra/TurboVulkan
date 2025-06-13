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

		[[nodiscard]] double GetDeltaTime() const { return mDeltaTime; }
		[[nodiscard]] double GetTimeFromEngineStart() const { return mTimeFromEngineStart; };

	protected:
		void Tick();

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
