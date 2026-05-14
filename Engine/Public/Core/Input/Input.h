#pragma once
#include "Core/Delegate.h"
#include "Layers/Event.h"

DECLARE_LOG_CATEGORY(LogInput, Display, Display)

namespace Turbo
{
	struct FKey
	{
		FName mKeyName;
		bool mbAxis;

		FKey()
		{
			mKeyName = {};
			mbAxis = false;
		}

		FKey(FName keyName, bool bAxis);
		FKey(const FKey& other);
		FKey& operator=(const FKey& other);

		friend bool operator==(const FKey& lhs, const FKey& rhs)
		{
			return lhs.mKeyName == rhs.mKeyName;
		}

		friend bool operator!=(const FKey& lhs, const FKey& rhs)
		{
			return !(lhs == rhs);
		}
	};

	enum class EKeyModifier : uint8
	{
		None = 0,
		LeftShift	= 1 << 0,
		RightShift	= 1 << 1,
		LeftCtrl	= 1 << 2,
		RightCtrl	= 1 << 3,
		LeftAlt		= 1 << 4,
		RightAlt	= 1 << 5,
	};

	DEFINE_ENUM_OPERATORS(EKeyModifier, uint8)

	struct FKeyEvent : FEventBase
	{
		EVENT_BODY(FKeyEvent)

		FKey mKey;

		bool mbDown : 1 = false;
		bool mbRepeat : 1 = false;
		EKeyModifier mModifiers : 5 = EKeyModifier::None;
	};

	struct FAxisEvent : FEventBase
	{
		EVENT_BODY(FAxisEvent)

		FKey mKey;

		float mValue = 0.f;
	};

	struct FActionEvent : FEventBase
	{
		EVENT_BODY(FActionEvent)

		FName mName{};

		FKey mKey{};
		bool mbAxis = false;

		bool mbDown = false;
		float mValue = 0.f;

		EKeyModifier mModifiers = EKeyModifier::None;

		FActionEvent(const FActionEvent& other);
		FActionEvent& operator=(const FActionEvent& other);
	};

	struct FActionBinding
	{
		FName mName;
		FKey mKey;
		EKeyModifier mRequiredModifiers = EKeyModifier::None;
	};

	DECLARE_MULTICAST_DELEGATE(FOnKeyEvent, FKeyEvent);
	DECLARE_MULTICAST_DELEGATE(FOnActionEvent, FActionEvent);

	class IInputSystem
	{
	public:
		virtual ~IInputSystem() = default;

	public:
		static IInputSystem* Get();

	public:
		virtual void Init() = 0;
		virtual void Destroy() = 0;

		virtual float GetAxisValue(const FKey& key) = 0;
		virtual float GetActionValue(FName actionName) = 0;

		virtual bool IsKeyPressed(const FKey& key) = 0;
		virtual bool IsActionPressed(FName actionName) = 0;

		virtual bool RegisterBinding(const FActionBinding& ActionBinding) = 0;
	};
} // Turbo
