#pragma once

#include "Layers/Event.h"

namespace Turbo
{
	enum class EGizmoOperation : uint8
	{
		Translate,
		Rotate,
		Scale
	};

	enum class EGizmoSpace : uint8
	{
		Local,
		World
	};

	class FEditorGizmo
	{
	public:
		void Init();

		void Draw();
		void HandleEvent(FEventBase& event);

	private:
		EGizmoOperation mGizmoOperation = EGizmoOperation::Translate;
		EGizmoSpace mGizmoSpace = EGizmoSpace::Local;
	};
} // Turbo
