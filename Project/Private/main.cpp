#include "Core/Engine.h"

int32_t main(int argc, char* argv[])
{
    Turbo::FEngine::Init();
    return Turbo::gEngine->Start(argc, argv);
}
