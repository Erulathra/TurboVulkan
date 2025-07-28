#pragma once
#include "Core/Delegate.h"

namespace Turbo
{
	struct FKey
	{
		FName keyName{};
		bool bAxis = false;

		FKey() = default;

		FKey(FName keyName, bool bAxis);
		FKey(const FKey& other);
		FKey& operator=(const FKey& other);

		friend bool operator==(const FKey& lhs, const FKey& rhs)
		{
			return lhs.keyName == rhs.keyName;
		}

		friend bool operator!=(const FKey& lhs, const FKey& rhs)
		{
			return !(lhs == rhs);
		}
	};

	struct FKeyEvent
	{
		FKey Key;

		bool bDown : 1 = false;
		bool bRepeat : 1 = false;
	};

	struct FAxisEvent
	{
		FKey Key;

		float Value = 0.f;
	};

	struct FActionEvent
	{
		FName ActionName{};

		FKey Key{};
		bool bAxis = false;

		bool bDown = false;
		float Value = 0.f;

		FActionEvent() = default;

		FActionEvent(const FActionEvent& other);
		FActionEvent& operator=(const FActionEvent& other);
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
		FOnKeyEvent OnKeyEvent;
		FOnActionEvent OnActionEvent;

	public:
		virtual void Init() = 0;
		virtual void Destroy() = 0;

		virtual float GetAxisValue(const FKey& key) = 0;
		virtual float GetActionValue(FName actionName) = 0;

		virtual bool IsKeyPressed(const FKey& key) = 0;
		virtual bool IsActionPressed(FName actionName) = 0;

		virtual FOnActionEvent* GetActionEvent(FName actionName) = 0;

		virtual bool RegisterBinding(FName actionName, const FKey& key) = 0;
		virtual std::unordered_map<FName, FKey>& GetBindings() = 0;
	};
} // Turbo
