#include "engine.h"

bool g_debug_arg {false};

int main(int /*argc*/, char** /*argv*/)
{
    RealmEngine::Engine engine;

    engine.boot();

    engine.run();

    engine.terminate();

    return 0;
}