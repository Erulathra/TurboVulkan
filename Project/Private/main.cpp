#include "Core/Engine.h"

int32_t main(int argc, char* argv[])
{
    Turbo::Engine::Init();
    return Turbo::gEngine->Start(argc, argv);
}
