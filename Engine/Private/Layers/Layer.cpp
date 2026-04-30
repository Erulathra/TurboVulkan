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
		mLayerLookUp.emplace(newLayer->GetName(), mLayers.size() - 1);
	}

	void FLayersStack::PopLayer()
	{
		mLayers.pop_back();
	}

	void FLayersStack::RemoveLayer(FName layerName)
	{
		if (auto foundIt = mLayerLookUp.find(layerName);
			foundIt != mLayerLookUp.end())
		{
			mLayers.erase(mLayers.begin() + foundIt->second);
			mLayerLookUp.erase(layerName);
		}
	}

	ILayer* FLayersStack::GetLayer(FName layerName)
	{
		if (auto foundIt = mLayerLookUp.find(layerName);
			foundIt != mLayerLookUp.end())
		{
			const uint32 layerIndex = foundIt->second;
			return mLayers[layerIndex].get();
		}

		return nullptr;
	}

} // Turbo