#include "Layers/Layer.h"

#include "Core/Engine.h"

namespace Turbo
{
	FLayersStack::FLayersStack()
	{
	}

	void FLayersStack::PushLayer_Impl(const TSharedPtr<ILayer>& newLayer)
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
		std::erase_if(mLayers, [layerName](const TSharedPtr<ILayer>& layer)
		{
			return layer->GetName() == layerName;
		});
	}

	TSharedPtr<ILayer> FLayersStack::GetLayer(FName layerName)
	{
		for (const TSharedPtr<ILayer>& layer : mLayers)
		{
			if (layer->GetName() == layerName)
			{
				return layer;
			}
		}

		return nullptr;
	}

} // Turbo