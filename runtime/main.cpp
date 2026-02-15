#include "engine.h"

int main(int /* argc */, char** /* argv */)
{
    RealmEngine::Engine engine;
    engine.initialize();
    engine.loop();
    engine.shutdown();

    return 0;
}
