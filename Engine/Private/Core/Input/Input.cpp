#include "Core/Input/Input.h"

namespace Turbo
{
	FKey::FKey(FName keyName, bool bAxis)
		: mKeyName(keyName)
		, mbAxis(bAxis)
	{
	}

	FKey::FKey(const FKey& other)
		: mKeyName(other.mKeyName)
		, mbAxis(other.mbAxis)
	{
	}

	FKey& FKey::operator=(const FKey& other)
	{
		if (this == &other)
			return *this;

		mKeyName = other.mKeyName;
		mbAxis = other.mbAxis;
		return *this;
	}

	FActionEvent::FActionEvent(const FActionEvent& other)
		: FEventBase(other.mEventTypeId)
		, mName(other.mName)
		, mKey(other.mKey)
		, mbAxis(other.mbAxis)
		, mbDown(other.mbDown)
		, mValue(other.mValue)
		, mModifiers(other.mModifiers)
	{
	}

	FActionEvent& FActionEvent::operator=(const FActionEvent& other)
	{
		if (this == &other)
		{
			return *this;
		}

		mName = other.mName;
		mKey = other.mKey;
		mbAxis = other.mbAxis;
		mbDown = other.mbDown;
		mValue = other.mValue;
		mModifiers = other.mModifiers;

		return *this;
	}
} // Turbo
