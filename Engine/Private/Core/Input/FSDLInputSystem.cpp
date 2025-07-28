#include "FSDLInputSystem.h"

#include "Core/Engine.h"
#include "Core/Input/Keys.h"

namespace Turbo
{
	void FSDLInputSystem::Init()
	{
		FSDLWindow* SDLWindow = reinterpret_cast<FSDLWindow*>(gEngine->GetWindow());
		SDLWindow->BindKeyboardEvent(FOnSDLKeyboardEvent::CreateRaw(this, &ThisClass::HandleSDLKeyboardEvent));
	}

	void FSDLInputSystem::Destroy()
	{
		FSDLWindow* SDLWindow = reinterpret_cast<FSDLWindow*>(gEngine->GetWindow());
		SDLWindow->RemoveKeyboardEvent();
	}

	float FSDLInputSystem::GetAxisValue(const FKey& key)
	{
		if (!key.bAxis)
		{
			return 0.f;
		}

		if (const auto axisValueIt = mLastAxisValues.find(key.keyName);
			axisValueIt != mLastAxisValues.end())
		{
			return axisValueIt->second;
		}

		return 0.f;
	}

	float FSDLInputSystem::GetActionValue(FName actionName)
	{
		if (const auto actionKeyIt = mActionBindings.find(actionName);
			actionKeyIt != mActionBindings.end())
		{
			if (actionKeyIt->second.bAxis)
			{
				return GetAxisValue(actionKeyIt->second);
			}

			return IsKeyPressed(actionKeyIt->second) ? 1.f : 0.f;
		}

		return 0.f;
	}

	bool FSDLInputSystem::IsKeyPressed(const FKey& key)
	{
		if (const auto actionEventIt = mLastKeyValues.find(key.keyName);
			actionEventIt != mLastKeyValues.end())
		{
			return actionEventIt->second;
		}

		return false;
	}

	bool FSDLInputSystem::IsActionPressed(FName actionName)
	{
		constexpr float kAnalogActivationValue = 0.25f;

		if (const auto actionKeyIt = mActionBindings.find(actionName);
			actionKeyIt != mActionBindings.end())
		{
			if (actionKeyIt->second.bAxis)
			{
				return GetAxisValue(actionKeyIt->second) > kAnalogActivationValue;
			}

			return IsKeyPressed(actionKeyIt->second);
		}

		return false;
	}

	FOnActionEvent* FSDLInputSystem::GetActionEvent(FName actionName)
	{
		FOnActionEvent* result = nullptr;

		if (const auto actionEventIt = mActionEventDelegates.find(actionName);
			actionEventIt != mActionEventDelegates.end())
		{
			result = &actionEventIt->second;
		}

		return result;
	}

	bool FSDLInputSystem::RegisterBinding(FName actionName, const FKey& key)
	{
		mActionBindings[actionName] = key;

		if (!mActionEventDelegates.contains(actionName))
		{
			mActionEventDelegates[actionName] = FOnActionEvent();
		}

		return true;
	}

	void FSDLInputSystem::HandleSDLKeyboardEvent(const SDL_KeyboardEvent& keyboardEvent)
	{
		TRACE_ZONE_SCOPED();

		const FKey key = ConvertSDLKey(keyboardEvent.key);
		if (key == EKeys::None)
		{
			TURBO_LOG(LOG_INPUT, Warn, "InputEvent: Unknown input code: {}", keyboardEvent.key);
			return;
		}

		if (!key.bAxis)
		{
			mLastKeyValues[key.keyName] = keyboardEvent.down;

			FKeyEvent newKeyEvent{};
			newKeyEvent.Key = key;
			newKeyEvent.bDown = keyboardEvent.down;
			newKeyEvent.bRepeat = keyboardEvent.repeat;

			OnKeyEvent.Broadcast(newKeyEvent);
			HandleKeyEvent(newKeyEvent);
		}
	}

	void FSDLInputSystem::HandleKeyEvent(const FKeyEvent& keyEvent)
	{
		TRACE_ZONE_SCOPED();

		if (keyEvent.bRepeat)
		{
			return;
		}

		for (const auto& [actionName, key] : mActionBindings)
		{
			if (key == keyEvent.Key)
			{
				FActionEvent newActionEvent{};
				newActionEvent.ActionName = actionName;
				newActionEvent.Key = keyEvent.Key;
				newActionEvent.bDown = keyEvent.bDown;
				newActionEvent.bAxis = false;

				OnActionEvent.Broadcast(newActionEvent);
				mActionEventDelegates[actionName].Broadcast(newActionEvent);
			}
		}
	}

	FKey FSDLInputSystem::ConvertSDLKey(SDL_Keycode key)
	{
		// TODO: Handle rest of the keyboard
		switch (key)
		{
		// Numbers
		case SDLK_0: return EKeys::Zero;
		case SDLK_1: return EKeys::One;
		case SDLK_2: return EKeys::Two;
		case SDLK_3: return EKeys::Three;
		case SDLK_4: return EKeys::Four;
		case SDLK_5: return EKeys::Five;
		case SDLK_6: return EKeys::Six;
		case SDLK_7: return EKeys::Seven;
		case SDLK_8: return EKeys::Eight;
		case SDLK_9: return EKeys::Nine;

		// Functional Keys
		case SDLK_F1: return EKeys::F1;
		case SDLK_F2: return EKeys::F2;
		case SDLK_F3: return EKeys::F3;
		case SDLK_F4: return EKeys::F4;
		case SDLK_F5: return EKeys::F5;
		case SDLK_F6: return EKeys::F6;
		case SDLK_F7: return EKeys::F7;
		case SDLK_F8: return EKeys::F8;
		case SDLK_F9: return EKeys::F9;
		case SDLK_F10: return EKeys::F10;
		case SDLK_F11: return EKeys::F11;
		case SDLK_F12: return EKeys::F12;

		default: return EKeys::None;
		}
	}
} // Turbo
