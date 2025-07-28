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
	} // namespace EKeys
} // namespace Turbo