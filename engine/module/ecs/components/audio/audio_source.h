#pragma once

#include <string>

namespace RealmEngine
{
    struct AudioSource
    {
        std::string clip_path;
        float       volume          = 1.0f;
        bool        loop            = false;
        bool        spatial         = true;
        bool        play_on_start   = false;
        bool        playing         = false;
        bool        start_attempted = false;
    };

} // namespace RealmEngine
