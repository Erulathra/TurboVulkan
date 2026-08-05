#include "RuntimeTestLayer.h"
#include "Core/Engine.h"
#include "Layers/Layer.h"
#include "Rendering/GameViewportLayer.h"

int32_t main(int argc, char* argv[])
{
    Turbo::FEngine::Init(argc, argv);

    Turbo::FLayersStack& layerStack = entt::locator<Turbo::FLayersStack>::value();
    layerStack.PushLayer<Turbo::FGameViewportLayer>();
    Turbo::gEngine->RegisterEngineLayers();

    layerStack.PushLayer<Turbo::FRuntimeTestLayer>();

    return Turbo::gEngine->Start();
}
