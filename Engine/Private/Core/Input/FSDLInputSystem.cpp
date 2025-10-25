#include "FSDLInputSystem.h"

#include "Core/Engine.h"
#include "Core/Input/Keys.h"

namespace Turbo
{
	void FSDLInputSystem::Init()
	{
		FWindow& SDLWindow = entt::locator<FWindow>::value();
		SDLWindow.BindKeyboardEvent(FOnSDLKeyboardEvent::CreateRaw(this, &ThisClass::HandleSDLKeyboardEvent));
	}

	void FSDLInputSystem::Destroy()
	{
		FWindow& SDLWindow = entt::locator<FWindow>::value();
		SDLWindow.RemoveKeyboardEvent();
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

	FOnActionEvent& FSDLInputSystem::GetActionEvent(FName actionName)
	{
		return mActionEventDelegates[actionName];
	}

	bool FSDLInputSystem::RegisterBinding(FName actionName, const FKey& key)
	{
		mActionBindings[actionName] = key;

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
		case SDLK_F13: return EKeys::F13;
		case SDLK_F14: return EKeys::F14;
		case SDLK_F15: return EKeys::F15;
		case SDLK_F16: return EKeys::F16;
		case SDLK_F17: return EKeys::F17;
		case SDLK_F18: return EKeys::F18;
		case SDLK_F19: return EKeys::F19;
		case SDLK_F20: return EKeys::F20;
		case SDLK_F21: return EKeys::F21;
		case SDLK_F22: return EKeys::F22;
		case SDLK_F23: return EKeys::F23;
		case SDLK_F24: return EKeys::F24;

		case SDLK_RETURN: return EKeys::Enter;
		case SDLK_ESCAPE: return EKeys::Escape;
		case SDLK_BACKSPACE: return EKeys::Backspace;
		case SDLK_TAB: return EKeys::Tab;
		case SDLK_SPACE: return EKeys::Space;
		case SDLK_EXCLAIM: return EKeys::Exclaim;
		case SDLK_DBLAPOSTROPHE: return EKeys::DoubleApostrophe;
		case SDLK_HASH: return EKeys::Hash;
		case SDLK_DOLLAR: return EKeys::Dollar;
		case SDLK_PERCENT: return EKeys::Percent;
		case SDLK_AMPERSAND: return EKeys::Ampersand;
		case SDLK_APOSTROPHE: return EKeys::Apostrophe;
		case SDLK_LEFTPAREN: return EKeys::LeftParenthesis;
		case SDLK_RIGHTPAREN: return EKeys::RightParenthesis;
		case SDLK_ASTERISK: return EKeys::Asterisk;
		case SDLK_PLUS: return EKeys::Plus;
		case SDLK_COMMA: return EKeys::Comma;
		case SDLK_MINUS: return EKeys::Minus;
		case SDLK_PERIOD: return EKeys::Period;
		case SDLK_SLASH: return EKeys::Slash;
		case SDLK_COLON: return EKeys::Colon;
		case SDLK_SEMICOLON: return EKeys::Semicolon;
		case SDLK_LESS: return EKeys::Less;
		case SDLK_EQUALS: return EKeys::Equals;
		case SDLK_GREATER: return EKeys::Greater;
		case SDLK_QUESTION: return EKeys::Question;
		case SDLK_AT: return EKeys::At;
		case SDLK_LEFTBRACKET: return EKeys::LeftBracket;
		case SDLK_BACKSLASH: return EKeys::Backslash;
		case SDLK_RIGHTBRACKET: return EKeys::Rightbracket;
		case SDLK_CARET: return EKeys::Caret;
		case SDLK_UNDERSCORE: return EKeys::Underscore;
		case SDLK_GRAVE: return EKeys::Grave;
		case SDLK_LEFTBRACE: return EKeys::LeftBrace;
		case SDLK_PIPE: return EKeys::Pipe;
		case SDLK_RIGHTBRACE: return EKeys::RightBrace;
		case SDLK_TILDE: return EKeys::Tilde;

		case SDLK_A: return EKeys::A;
		case SDLK_B: return EKeys::B;
		case SDLK_C: return EKeys::C;
		case SDLK_D: return EKeys::D;
		case SDLK_E: return EKeys::E;
		case SDLK_F: return EKeys::F;
		case SDLK_G: return EKeys::G;
		case SDLK_H: return EKeys::H;
		case SDLK_I: return EKeys::I;
		case SDLK_J: return EKeys::J;
		case SDLK_K: return EKeys::K;
		case SDLK_L: return EKeys::L;
		case SDLK_M: return EKeys::M;
		case SDLK_N: return EKeys::N;
		case SDLK_O: return EKeys::O;
		case SDLK_P: return EKeys::P;
		case SDLK_Q: return EKeys::Q;
		case SDLK_R: return EKeys::R;
		case SDLK_S: return EKeys::S;
		case SDLK_T: return EKeys::T;
		case SDLK_U: return EKeys::U;
		case SDLK_V: return EKeys::V;
		case SDLK_W: return EKeys::W;
		case SDLK_X: return EKeys::X;
		case SDLK_Y: return EKeys::Y;
		case SDLK_Z: return EKeys::Z;

		case SDLK_DELETE: return EKeys::Delete;
		case SDLK_PLUSMINUS: return EKeys::PlusMinus;
		case SDLK_CAPSLOCK: return EKeys::Capslock;
		case SDLK_PRINTSCREEN: return EKeys::PrintScreen;
		case SDLK_SCROLLLOCK: return EKeys::Scrolllock;
		case SDLK_PAUSE: return EKeys::Pause;
		case SDLK_INSERT: return EKeys::Insert;
		case SDLK_HOME: return EKeys::Home;
		case SDLK_PAGEUP: return EKeys::PageUp;
		case SDLK_END: return EKeys::End;
		case SDLK_PAGEDOWN: return EKeys::PageDown;

		case SDLK_RIGHT: return EKeys::Right;
		case SDLK_LEFT: return EKeys::Left;
		case SDLK_DOWN: return EKeys::Down;
		case SDLK_UP: return EKeys::Up;

		case SDLK_NUMLOCKCLEAR: return EKeys::NumlockClear;
		case SDLK_KP_DIVIDE: return EKeys::NumDivide;
		case SDLK_KP_MULTIPLY: return EKeys::NumMultiply;
		case SDLK_KP_MINUS: return EKeys::NumMinus;
		case SDLK_KP_PLUS: return EKeys::NumPlus;
		case SDLK_KP_ENTER: return EKeys::NumEnter;
		case SDLK_KP_1: return EKeys::NumOne;
		case SDLK_KP_2: return EKeys::NumTwo;
		case SDLK_KP_3: return EKeys::NumThree;
		case SDLK_KP_4: return EKeys::NumFour;
		case SDLK_KP_5: return EKeys::NumFive;
		case SDLK_KP_6: return EKeys::NumSix;
		case SDLK_KP_7: return EKeys::NumSeven;
		case SDLK_KP_8: return EKeys::NumEight;
		case SDLK_KP_9: return EKeys::NumNine;
		case SDLK_KP_0: return EKeys::NumZero;
		case SDLK_KP_PERIOD: return EKeys::NumPeriod;

		case SDLK_LCTRL: return EKeys::LeftCtrl;
		case SDLK_LSHIFT: return EKeys::LeftShift;
		case SDLK_LALT: return EKeys::LeftAlt;
		case SDLK_LGUI: return EKeys::LeftGui;

		case SDLK_RCTRL: return EKeys::RightCtrl;
		case SDLK_RSHIFT: return EKeys::RightShift;
		case SDLK_RALT: return EKeys::RightAlt;
		case SDLK_MODE: return EKeys::RightAlt;
		case SDLK_RGUI: return EKeys::RightGui;

		default: return EKeys::None;
		}
	}
} // Turbo
