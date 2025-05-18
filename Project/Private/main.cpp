#include "Core/Engine.h"

int32_t main(int argc, char* argv[])
{
    Turbo::Engine::Init();
    return Turbo::Engine::Get()->Start(argc, argv);
}
