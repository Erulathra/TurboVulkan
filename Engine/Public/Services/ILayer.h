#pragma once
#include "Core/Delegate.h"
#include "Graphics/Resources.h"

namespace Turbo
{
	class FCommandBuffer;
	class FGPUDevice;

	class ILayer
	{
		/** Interface */
	public:
		virtual ~ILayer() = default;

		virtual void Start() = 0;
		virtual void Shutdown() = 0;

		virtual void BeginTick_GameThread(double deltaTime) {};
		virtual void EndTick_GameThread(double deltaTime) {};

		virtual void PostBeginFrame_RenderThread(FGPUDevice* gpu, FCommandBuffer* cmd) {};
		virtual void BeginPresentingFrame_RenderThread(FGPUDevice* gpu, FCommandBuffer* cmd, THandle<FTexture> presentImage) {};

		virtual bool ShouldTick() { return false; };
		virtual bool ShouldRender() { return false; };

		virtual FName GetName() = 0;

		/** Interface end */
	};

	using FLayersCollection = std::vector<TSharedPtr<ILayer>>;

	class FLayersStack final
	{
	public:
		FLayersStack();
		DELETE_COPY(FLayersStack)

	public:
		template <typename ServiceType> requires std::is_base_of_v<ILayer, ServiceType>
		void PushLayer();

		void PopLayer();
		void RemoveLayer(FName layerName);

		TSharedPtr<ILayer> GetLayer(FName layerName);

		FLayersCollection::iterator begin() { return mLayers.begin(); };
		FLayersCollection::iterator end() { return mLayers.end(); };

		FLayersCollection::reverse_iterator rbegin() { return mLayers.rbegin(); };
		FLayersCollection::reverse_iterator rend() { return mLayers.rend(); };

	private:
		void PushLayer_Impl(const TSharedPtr<ILayer>& newLayer);

	private:
		FLayersCollection mLayers;
	};

	template <typename ServiceType> requires std::is_base_of_v<ILayer, ServiceType>
	void FLayersStack::PushLayer()
	{
		PushLayer_Impl(std::make_shared<ServiceType>());
	}
} // Turbo