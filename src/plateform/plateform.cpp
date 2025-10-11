#include "plateform.h"
#include <filesystem>

namespace RealmEngine
{
    std::filesystem::path PlateForm::getExecutablePath() noexcept
    {
#ifdef __linux__
        char    buffer[PATH_MAX];
        ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
        if (len != -1)
        {
            buffer[len] = '\0';
            return std::filesystem::path(buffer);
        }
#elif _WIN32
        char buffer[MAX_PATH];
        GetModuleFileNameA(NULL, buffer, MAX_PATH);
        return std::filesystem::path(buffer);
#elif __APPLE__
        char     buffer[PATH_MAX];
        uint32_t size = sizeof(buffer);
        if (_NSGetExecutablePath(buffer, &size) == 0)
        {
            return std::filesystem::path(buffer);
        }
#endif
        return std::filesystem::current_path() / "RealmEngine";
    }
} // namespace RealmEngine