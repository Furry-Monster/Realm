#include "editor.h"

int main(int /* argc */, char** /* argv */)
{
    RealmEngine::Editor editor;
    editor.initialize();
    editor.run();
    editor.shutdown();

    return 0;
}
