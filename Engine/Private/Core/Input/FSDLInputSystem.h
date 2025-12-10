#pragma once

#include "Core/Input/Input.h"
#include "SDL3/SDL_keycode.h"

struct SDL_KeyboardEvent;

namespace Turbo
{
	class FSDLInputSystem final : public IInputSystem
	{
		GENERATED_BODY(FSDLInputSystem, IInputSystem)

	private:
		FSDLInputSystem() = default;

	public:
		virtual ~FSDLInputSystem() override = default;

	public:
		virtual void Init() override;
		virtual void Destroy() override;

		virtual float GetAxisValue(const FKey& key) override;
		virtual float GetActionValue(FName actionName) override;

		virtual bool IsKeyPressed(const FKey& key) override;
		virtual bool IsActionPressed(FName actionName) override;

		virtual bool RegisterBinding(FName actionName, const FKey& key) override;
		virtual std::unordered_map<FName, FKey>& GetBindings() override { return mActionBindings; }

	private:
		void HandleSDLKeyboardEvent(const SDL_KeyboardEvent& keyboardEvent);
		static FKey ConvertSDLKey(SDL_Keycode key);

		void HandleKeyEvent(const FKeyEvent& keyEvent);

	private:
		std::unordered_map<FName /** actionName **/, FKey> mActionBindings;

		std::unordered_map<FName /** keyName **/, float> mLastAxisValues;
		std::unordered_map<FName /** keyName **/, bool> mLastKeyValues;

	public:
		friend class FEngine;
		friend class FWindow;
	};
} // Turbo
