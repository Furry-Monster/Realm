#include "editor.h"

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv)
{
    RealmEngine::Editor editor;
    editor.initialize();
    editor.run();
    editor.shutdown();

    return 0;
}
