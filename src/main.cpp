#include <string>
#include "editor/editor.h"
#include "engine.h"

int main(int argc, char** argv)
{
    if (argc > 1 && std::string(argv[1]) == "debug")
    {
        RealmEngine::Engine engine;
        engine.boot();
        engine.debug();
        engine.terminate();
    }
    else
    {
        RealmEngine::Editor editor;
        editor.initialize();
        editor.run();
        editor.shutdown();
    }

    return 0;
}
