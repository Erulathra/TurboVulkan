#include "Services/IService.h"

#include "Core/Engine.h"

namespace Turbo
{
	FLayersStack::FLayersStack()
	{
	}

	FLayersStack* FLayersStack::Get()
	{
		static FLayersStack* serviceManagerInstance = new FLayersStack();
		return serviceManagerInstance;
	}

	void FLayersStack::PushLayer_Impl(const ILayerPtr& newLayer)
	{
		TURBO_CHECK(gEngine->GetEngineState() <= EEngineState::Initializing)
		TURBO_CHECK(newLayer)

		TURBO_CHECK(GetLayer(newLayer->GetName()) == nullptr)

		mLayers.push_back(newLayer);
	}

	void FLayersStack::PopLayer()
	{
		mLayers.pop_back();
	}

	void FLayersStack::RemoveLayer(FName layerName)
	{
		std::erase_if(mLayers, [layerName](const ILayerPtr& layer)
		{
			return layer->GetName() == layerName;
		});
	}

	ILayerPtr FLayersStack::GetLayer(FName layerName)
	{
		for (const ILayerPtr& layer : mLayers)
		{
			if (layer->GetName() == layerName)
			{
				return layer;
			}
		}

		return nullptr;
	}
} // Turbo