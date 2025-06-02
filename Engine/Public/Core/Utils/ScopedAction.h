#pragma once

namespace Turbo
{
	template <typename OnExitScope>
	class TScopedAction
	{
	private:
		auto operator=(TScopedAction const&) = delete;

	public:
		constexpr TScopedAction() = default;
		constexpr ~TScopedAction()
		{
			OnExitScope();
		}
	};
} // Turbo
