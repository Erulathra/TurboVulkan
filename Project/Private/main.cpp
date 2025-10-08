#include "RenderingTestLayer.h"
#include "Core/Engine.h"
#include "Services/ILayer.h"

int32_t main(int argc, char* argv[])
{
    Turbo::FEngine::Init();
    Turbo::FLayersStack::Get()->PushLayer<FRenderingTestLayer>();

    return Turbo::gEngine->Start(argc, argv);
}
