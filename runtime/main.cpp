#include "engine.h"

int main(int /* argc */, char** /* argv */)
{
    RealmEngine::Engine engine;
    engine.boot();
    engine.debug();
    engine.shutdown();

    return 0;
}
