#include "engine.h"

int main()
{
    RealmEngine::Engine engine;

    engine.boot();
    engine.run();
    engine.terminate();

    return 0;
}