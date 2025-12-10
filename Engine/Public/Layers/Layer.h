#pragma once
#include "Event.h"
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

		virtual void OnEvent(FEventBase& event) {}

		virtual void BeginTick(double deltaTime) {};
		virtual void EndTick(double deltaTime) {};

		virtual void PostBeginFrame(FGPUDevice* gpu, FCommandBuffer* cmd) {};
		virtual void RenderScene(FGPUDevice* gpu, FCommandBuffer* cmd) {};
		virtual void BeginPresentingFrame(FGPUDevice* gpu, FCommandBuffer* cmd, THandle<FTexture> presentImage) {};

		virtual bool ShouldTick() { return false; };
		virtual bool ShouldRender() { return false; };

		virtual FName GetName() = 0;

		/** Interface end */
	};

	using FLayersCollection = std::vector<TSharedPtr<ILayer>>;

	class FLayersStack final
	{
	public:
		using Iterator = FLayersCollection::iterator;
		using ReverseIterator = FLayersCollection::reverse_iterator;

	public:
		FLayersStack();
		DELETE_COPY(FLayersStack)

	public:
		template <typename ServiceType> requires std::is_base_of_v<ILayer, ServiceType>
		void PushLayer();

		void PopLayer();
		void RemoveLayer(FName layerName);

		TSharedPtr<ILayer> GetLayer(FName layerName);

		Iterator begin() { return mLayers.begin(); };
		Iterator end() { return mLayers.end(); };

		ReverseIterator rbegin() { return mLayers.rbegin(); };
		ReverseIterator rend() { return mLayers.rend(); };

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