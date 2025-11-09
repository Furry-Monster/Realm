#pragma once

#ifdef __linux__
#include <limits.h>
#include <unistd.h>
#elif _WIN32
#include <windows.h>
#elif __APPLE__
#include <mach-o/dyld.h>
#endif

#include <filesystem>

namespace RealmEngine
{
    class Plateform
    {
    public:
        static std::filesystem::path getExecutablePath() noexcept;
    };
} // namespace RealmEngine