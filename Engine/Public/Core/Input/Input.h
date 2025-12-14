#pragma once
#include "Core/Delegate.h"
#include "Layers/Event.h"

namespace Turbo
{
	struct FKey
	{
		FName mKeyName{};
		bool mbAxis = false;

		FKey() = default;

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

	struct FKeyEvent : FEventBase
	{
		EVENT_BODY(FKeyEvent)

		FKey mKey;

		bool mbDown : 1 = false;
		bool mbRepeat : 1 = false;
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

		FName mActionName{};

		FKey mKey{};
		bool mbAxis = false;

		bool mbDown = false;
		float mValue = 0.f;

		FActionEvent(const FActionEvent& other);
		FActionEvent& operator=(const FActionEvent& other);
	};

	struct FActionBinding
	{
		FName mActionName;
		FKey mKey;
	};

	DECLARE_MULTICAST_DELEGATE(FOnKeyEvent, FKeyEvent);
	DECLARE_MULTICAST_DELEGATE(FOnActionEvent, FActionEvent);

	class IInputSystem
	{
		GENERATED_BODY(IInputSystem)

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
		virtual std::unordered_map<FName, FKey>& GetBindings() = 0;
	};
} // Turbo
