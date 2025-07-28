#include "Core/Input/Input.h"

namespace Turbo
{
	FKey::FKey(FName keyName, bool bAxis)
		: keyName(keyName)
		, bAxis(bAxis)
	{
	}

	FKey::FKey(const FKey& other)
		: keyName(other.keyName)
		, bAxis(other.bAxis)
	{
	}

	FKey& FKey::operator=(const FKey& other)
	{
		if (this == &other)
			return *this;

		keyName = other.keyName;
		bAxis = other.bAxis;
		return *this;
	}

	FActionEvent::FActionEvent(const FActionEvent& other)
		: ActionName(other.ActionName)
		, Key(other.Key)
		, bAxis(other.bAxis)
		, bDown(other.bDown)
		, Value(other.Value)
	{
	}

	FActionEvent& FActionEvent::operator=(const FActionEvent& other)
	{
		if (this == &other)
			return *this;

		ActionName = other.ActionName;
		Key = other.Key;
		bAxis = other.bAxis;
		bDown = other.bDown;
		Value = other.Value;

		return *this;
	}
} // Turbo
