#pragma once
#include "Event.h"
#include "Core/Delegate.h"
#include "Graphics/FrameGraph/RenderGraph.h"
#include "Graphics/FrameGraph/RenderGraphHelpers.h"

namespace Turbo
{
	struct FRGResourceHandle;
	class FCommandBuffer;
	class FGPUDevice;

	class ILayer
	{
		/** Interface */
	public:
		virtual ~ILayer() = default;

		virtual void PreGPUInit() {};

		virtual void Start() = 0;
		virtual void Shutdown() = 0;

		virtual void OnEvent(FEventBase& event) {}

		virtual void BeginTick(double deltaTime) {};
		virtual void EndTick(double deltaTime) {};

		virtual void PostBeginFrame(FRenderGraphBuilder& graphBuilder) {};
		virtual void EndFrame(FRenderGraphBuilder& graphBuilder, FRGResourceHandle presentTexture) {};
		virtual void BeginPresentingFrame(FRenderGraphBuilder& graphBuilder, FRGResourceHandle presentTexture) {};

		virtual bool ShouldTick() { return false; };
		virtual bool ShouldRender() { return false; };

		virtual FName GetName() = 0;

		/** Interface end */
	};

	template<typename LayerType>
		requires std::is_base_of_v<ILayer, LayerType>
	FName GetStaticLayerName() = delete;

	using FLayersCollection = std::vector<TSharedPtr<ILayer>>;
	using FLayerLookUp = entt::dense_map<FName, uint32>;

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

		template<typename LayerType>
			requires std::is_base_of_v<ILayer, LayerType>
		void RemoveLayer()
		{
			RemoveLayer(GetStaticLayerName<LayerType>());
		}

		ILayer* GetLayer(FName layerName);

		template<typename LayerType>
			requires std::is_base_of_v<ILayer, LayerType>
		LayerType* GetLayer()
		{
			return static_cast<LayerType*>(GetLayer(GetStaticLayerName<LayerType>()));
		}

		template<typename LayerType>
			requires std::is_base_of_v<ILayer, LayerType>
		LayerType* GetLayerChecked()
		{
			LayerType* result = static_cast<LayerType*>(GetLayer(GetStaticLayerName<LayerType>()));
			TURBO_CHECK(result)
			return result;
		}

		Iterator begin() { return mLayers.begin(); };
		Iterator end() { return mLayers.end(); };

		ReverseIterator rbegin() { return mLayers.rbegin(); };
		ReverseIterator rend() { return mLayers.rend(); };

	private:
		void PushLayer_Impl(const TSharedPtr<ILayer>& newLayer);

	private:
		FLayersCollection mLayers;
		FLayerLookUp mLayerLookUp;
	};

	template <typename ServiceType> requires std::is_base_of_v<ILayer, ServiceType>
	void FLayersStack::PushLayer()
	{
		PushLayer_Impl(std::make_shared<ServiceType>());
	}
} // Turbo