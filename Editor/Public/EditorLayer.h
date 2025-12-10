#pragma once

#include "Core/WindowEvents.h"
#include "Core/Input/Input.h"
#include "Layers/Layer.h"

namespace Turbo
{
	class FEditorLayer : public ILayer
	{
	public:
		virtual void Start() override;
		virtual void Shutdown() override;

		virtual void OnEvent(FEventBase& event) override;

	public:
		virtual FName GetName() override;

	private:
		void HandleInputActionEvent(FActionEvent& event);
		void HandleCloseEvent(FCloseWindowEvent& event);
	};
} // Turbo
