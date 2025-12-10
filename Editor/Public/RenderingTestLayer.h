#pragma once

#include "Layers/Layer.h"

namespace Turbo
{
	class FRenderingTestLayer final : public Turbo::ILayer
	{
		/** IService Interface */
	public:
		virtual ~FRenderingTestLayer() override;
		FRenderingTestLayer();

		virtual void Start() override;
		virtual void Shutdown() override;
		void ShowImGuiWindow();

		virtual void BeginTick(double deltaTime) override;

		virtual bool ShouldTick() override { return true; }
		virtual bool ShouldRender() override { return true; }

		virtual Turbo::FName GetName() override;

		/** IService Interface end */

	private:
		entt::entity mCameraEntity = entt::null;
	};

}
