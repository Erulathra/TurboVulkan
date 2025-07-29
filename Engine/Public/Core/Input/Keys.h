#pragma once

#include "Input.h"

#define DECLARE_KEY(KeyName, bIsAxis) const inline FKey KeyName{FName(#KeyName), bIsAxis}

namespace Turbo
{
	namespace EKeys
	{
		const inline FKey None{};

		// Numbers
		DECLARE_KEY(Zero, false);
		DECLARE_KEY(One, false);
		DECLARE_KEY(Two, false);
		DECLARE_KEY(Three, false);
		DECLARE_KEY(Four, false);
		DECLARE_KEY(Five, false);
		DECLARE_KEY(Six, false);
		DECLARE_KEY(Seven, false);
		DECLARE_KEY(Eight, false);
		DECLARE_KEY(Nine, false);

		// Letters
		DECLARE_KEY(A, false);
		DECLARE_KEY(B, false);
		DECLARE_KEY(C, false);
		DECLARE_KEY(D, false);
		DECLARE_KEY(E, false);
		DECLARE_KEY(F, false);
		DECLARE_KEY(G, false);
		DECLARE_KEY(H, false);
		DECLARE_KEY(I, false);
		DECLARE_KEY(J, false);
		DECLARE_KEY(K, false);
		DECLARE_KEY(L, false);
		DECLARE_KEY(M, false);
		DECLARE_KEY(N, false);
		DECLARE_KEY(O, false);
		DECLARE_KEY(P, false);
		DECLARE_KEY(Q, false);
		DECLARE_KEY(R, false);
		DECLARE_KEY(S, false);
		DECLARE_KEY(T, false);
		DECLARE_KEY(U, false);
		DECLARE_KEY(V, false);
		DECLARE_KEY(W, false);
		DECLARE_KEY(X, false);
		DECLARE_KEY(Y, false);
		DECLARE_KEY(Z, false);

		// Function
		DECLARE_KEY(F1, false);
		DECLARE_KEY(F2, false);
		DECLARE_KEY(F3, false);
		DECLARE_KEY(F4, false);
		DECLARE_KEY(F5, false);
		DECLARE_KEY(F6, false);
		DECLARE_KEY(F7, false);
		DECLARE_KEY(F8, false);
		DECLARE_KEY(F9, false);
		DECLARE_KEY(F10, false);
		DECLARE_KEY(F11, false);
		DECLARE_KEY(F12, false);

		DECLARE_KEY(F13, false);
		DECLARE_KEY(F14, false);
		DECLARE_KEY(F15, false);
		DECLARE_KEY(F16, false);
		DECLARE_KEY(F17, false);
		DECLARE_KEY(F18, false);
		DECLARE_KEY(F19, false);
		DECLARE_KEY(F20, false);
		DECLARE_KEY(F21, false);
		DECLARE_KEY(F22, false);
		DECLARE_KEY(F23, false);
		DECLARE_KEY(F24, false);

		DECLARE_KEY(Enter, false);
		DECLARE_KEY(Escape, false);
		DECLARE_KEY(Backspace, false);
		DECLARE_KEY(Tab, false);
		DECLARE_KEY(Space, false);
		DECLARE_KEY(Exclaim, false);
		DECLARE_KEY(DoubleApostrophe, false);
		DECLARE_KEY(Hash, false);
		DECLARE_KEY(Dollar, false);
		DECLARE_KEY(Percent, false);
		DECLARE_KEY(Ampersand, false);
		DECLARE_KEY(Apostrophe, false);
		DECLARE_KEY(LeftParenthesis, false);
		DECLARE_KEY(RightParenthesis, false);
		DECLARE_KEY(Asterisk, false);
		DECLARE_KEY(Plus, false);
		DECLARE_KEY(Comma, false);
		DECLARE_KEY(Minus, false);
		DECLARE_KEY(Period, false);
		DECLARE_KEY(Slash, false);
		DECLARE_KEY(Colon, false);
		DECLARE_KEY(Semicolon, false);
		DECLARE_KEY(Less, false);
		DECLARE_KEY(Equals, false);
		DECLARE_KEY(Greater, false);
		DECLARE_KEY(Question, false);
		DECLARE_KEY(At, false);
		DECLARE_KEY(LeftBracket, false);
		DECLARE_KEY(Rightbracket, false);
		DECLARE_KEY(Backslash, false);
		DECLARE_KEY(Caret, false);
		DECLARE_KEY(Underscore, false);
		DECLARE_KEY(Grave, false);
		DECLARE_KEY(LeftBrace, false);
		DECLARE_KEY(RightBrace, false);
		DECLARE_KEY(Pipe, false);
		DECLARE_KEY(Tilde, false);
		DECLARE_KEY(Delete, false);
		DECLARE_KEY(PlusMinus, false);

		DECLARE_KEY(Capslock, false);
		DECLARE_KEY(PrintScreen, false);
		DECLARE_KEY(Scrolllock, false);
		DECLARE_KEY(Pause, false);
		DECLARE_KEY(Insert, false);
		DECLARE_KEY(Home, false);
		DECLARE_KEY(PageUp, false);
		DECLARE_KEY(PageDown, false);
		DECLARE_KEY(End, false);

		DECLARE_KEY(Right, false);
		DECLARE_KEY(Left, false);
		DECLARE_KEY(Down, false);
		DECLARE_KEY(Up, false);

		DECLARE_KEY(NumlockClear, false);
		DECLARE_KEY(NumDivide, false);
		DECLARE_KEY(NumMultiply, false);
		DECLARE_KEY(NumMinus, false);
		DECLARE_KEY(NumPlus, false);
		DECLARE_KEY(NumEnter, false);
		DECLARE_KEY(NumOne, false);
		DECLARE_KEY(NumTwo, false);
		DECLARE_KEY(NumThree, false);
		DECLARE_KEY(NumFour, false);
		DECLARE_KEY(NumFive, false);
		DECLARE_KEY(NumSix, false);
		DECLARE_KEY(NumSeven, false);
		DECLARE_KEY(NumEight, false);
		DECLARE_KEY(NumNine, false);
		DECLARE_KEY(NumZero, false);
		DECLARE_KEY(NumPeriod, false);
		DECLARE_KEY(NumComma, false);

		DECLARE_KEY(LeftCtrl, false);
		DECLARE_KEY(LeftShift, false);
		DECLARE_KEY(LeftAlt, false);
		DECLARE_KEY(LeftGui, false);

		DECLARE_KEY(RightCtrl, false);
		DECLARE_KEY(RightShift, false);
		DECLARE_KEY(RightAlt, false);
		DECLARE_KEY(RightGui, false);
	} // namespace EKeys
} // namespace Turbo
