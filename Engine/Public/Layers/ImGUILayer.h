#pragma once

#include "Layer.h"
#include "SDL3/SDL_events.h"

namespace ImGui
{
	using FTextureId = uint32_t;

	void Texture(Turbo::THandle<Turbo::FTexture> textureHandle);
}

namespace Turbo
{
struct FImGuiTexture
	{
		THandle<FTexture> mTexture;
		FRGResourceHandle mRGTexture = {};
		vk::DescriptorSet mDescriptorSet = {};
	};

	class FImGuiLayer final : public ILayer
	{
	public:
		FImGuiTexture& FindOrRegisterTexture(THandle<FTexture> textureHandle);

		/** Service Api */
	public:
		virtual void Start() override;
		virtual void Shutdown() override;

		virtual void BeginTick(double deltaTime) override;
		virtual void EndTick(double deltaTime) override;
		virtual bool ShouldTick() override { return true; }

		virtual void PostBeginFrame(FRenderGraphBuilder& graphBuilder) override;
		virtual void BeginPresentingFrame(FRenderGraphBuilder& graphBuilder, FRGResourceHandle presentImage) override;
		virtual bool ShouldRender() override { return true; }

		virtual FName GetName() override;
		/** Service Api end */

	private:
		void OnSDLEvent(SDL_Event* sdlEvent);
		void SetupTheme();

	private:
		std::vector<FImGuiTexture> mTextures;
	};

} // Turbo