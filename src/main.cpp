#include "engine.h"

int main(int, char**)
{
    RealmEngine::Engine engine;

    engine.boot();
    engine.run();
    engine.terminate();

    return 0;
}