#include "engine.h"

int main(int /*argc*/, char** /*argv*/)
{
    RealmEngine::Engine engine;

    engine.boot();

    engine.debugRun();

    engine.terminate();

    return 0;
}