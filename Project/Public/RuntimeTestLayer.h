#pragma once
#include "Core/Input/Input.h"
#include "Layers/Layer.h"

namespace Turbo
{
	class FRuntimeTestLayer : public ILayer
	{
	public:
		virtual void Start() override;
		virtual void Shutdown() override;

		virtual void OnEvent(FEventBase& event) override;
		virtual void BeginTick(double deltaTime) override;
		virtual bool ShouldTick() override;

		virtual FName GetName() override;
	};
} // Turbo
