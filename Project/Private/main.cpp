#include "RenderingTestLayer.h"
#include "Core/Engine.h"
#include "Layers/Layer.h"

int32_t main(int argc, char* argv[])
{
    Turbo::FEngine::Init();

    Turbo::FLayersStack& layerStack = entt::locator<Turbo::FLayersStack>::value();

    return Turbo::gEngine->Start(argc, argv);
}
