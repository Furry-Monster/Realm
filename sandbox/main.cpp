#include "engine.h"

#include <iostream>

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv)
{
    try
    {
        RealmEngine::Engine engine;
        engine.initialize();
        engine.loop();
        engine.shutdown();
        return 0;
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Engine runtime failed: " << ex.what() << std::endl;
        return 1;
    }
    catch (...)
    {
        std::cerr << "Engine runtime failed: unknown error" << std::endl;
        return 1;
    }
}
