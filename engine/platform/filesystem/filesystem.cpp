#include "platform/filesystem/filesystem.h"

#include "core/log/log_macros.h"

#include <chrono>
#include <fstream>

#ifdef __linux__
#  include <limits.h>
#  include <unistd.h>
#elif defined(_WIN32)
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <windows.h>
#elif defined(__APPLE__)
#  include <limits.h>
#  include <mach-o/dyld.h>
#endif

namespace RealmEngine
{
    // platform-specific executable path

    std::filesystem::path FileSystem::getExecutablePath() noexcept
    {
#ifdef __linux__
        char    buffer[PATH_MAX];
        ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
        if (len != -1)
        {
            buffer[len] = '\0';
            return std::filesystem::path(buffer);
        }
        RE_LOG_WARN("readlink(/proc/self/exe) failed, falling back to cwd");
#elif defined(_WIN32)
        wchar_t buffer[MAX_PATH];
        DWORD   len = GetModuleFileNameW(NULL, buffer, MAX_PATH);
        if (len > 0 && len < MAX_PATH)
            return std::filesystem::path(buffer);

        // Retry with a larger buffer for long paths
        if (len == MAX_PATH || GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
            std::vector<wchar_t> big_buffer(32768);
            len = GetModuleFileNameW(NULL, big_buffer.data(), static_cast<DWORD>(big_buffer.size()));
            if (len > 0 && len < big_buffer.size())
                return std::filesystem::path(big_buffer.data(), big_buffer.data() + len);
        }
        RE_LOG_WARN("GetModuleFileNameW failed, falling back to cwd");
#elif defined(__APPLE__)
        char     buffer[PATH_MAX];
        uint32_t size = sizeof(buffer);
        if (_NSGetExecutablePath(buffer, &size) == 0)
        {
            // _NSGetExecutablePath may return a path with symlinks; resolve them
            std::error_code ec;
            auto            resolved = std::filesystem::canonical(buffer, ec);
            return ec ? std::filesystem::path(buffer) : resolved;
        }
        RE_LOG_WARN("_NSGetExecutablePath failed, falling back to cwd");
#else
        RE_LOG_WARN("Unsupported platform for getExecutablePath(), using cwd");
#endif
        // Last resort: return current working directory (without a hardcoded name)
        return std::filesystem::current_path();
    }

    std::filesystem::path FileSystem::getExecutableDir() noexcept { return getExecutablePath().parent_path(); }

    // file read

    std::optional<std::string> FileSystem::readTextFile(const std::filesystem::path& path)
    {
        // binary mode avoids platform-specific line-ending conversion (Windows CRLF)
        std::ifstream file(path, std::ios::in | std::ios::binary);
        if (!file.is_open())
        {
            RE_LOG_ERROR("Failed to open text file: " + path.string());
            return std::nullopt;
        }

        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        return content;
    }

    std::optional<std::vector<uint8_t>> FileSystem::readBinaryFile(const std::filesystem::path& path)
    {
        std::ifstream file(path, std::ios::in | std::ios::binary | std::ios::ate);
        if (!file.is_open())
        {
            RE_LOG_ERROR("Failed to open binary file: " + path.string());
            return std::nullopt;
        }

        auto size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<uint8_t> data(static_cast<size_t>(size));
        if (size > 0 && !file.read(reinterpret_cast<char*>(data.data()), size))
        {
            RE_LOG_ERROR("Failed to read binary file: " + path.string());
            return std::nullopt;
        }
        return data;
    }

    // file write

    bool FileSystem::writeTextFile(const std::filesystem::path& path, const std::string& content)
    {
        if (path.has_parent_path())
            createDirectories(path.parent_path());

        // binary mode keeps LF line endings on all platforms
        std::ofstream file(path, std::ios::out | std::ios::binary | std::ios::trunc);
        if (!file.is_open())
        {
            RE_LOG_ERROR("Failed to write text file: " + path.string());
            return false;
        }

        file << content;
        return file.good();
    }

    bool FileSystem::writeBinaryFile(const std::filesystem::path& path, const std::vector<uint8_t>& data)
    {
        if (path.has_parent_path())
            createDirectories(path.parent_path());

        std::ofstream file(path, std::ios::out | std::ios::binary | std::ios::trunc);
        if (!file.is_open())
        {
            RE_LOG_ERROR("Failed to write binary file: " + path.string());
            return false;
        }

        file.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
        return file.good();
    }

    // file queries

    std::uintmax_t FileSystem::getFileSize(const std::filesystem::path& path)
    {
        std::error_code ec;
        auto            size = std::filesystem::file_size(path, ec);
        if (ec)
            return 0;
        return size;
    }

    std::vector<std::filesystem::path> FileSystem::listFiles(const std::filesystem::path& directory,
                                                             const std::string&           extension)
    {
        std::vector<std::filesystem::path> results;
        std::error_code                    ec;

        if (!std::filesystem::is_directory(directory, ec))
            return results;

        for (const auto& entry : std::filesystem::directory_iterator(directory, ec))
        {
            if (!entry.is_regular_file())
                continue;
            if (!extension.empty() && entry.path().extension() != extension)
                continue;
            results.push_back(entry.path());
        }
        return results;
    }

    std::vector<std::filesystem::path> FileSystem::listFilesRecursive(const std::filesystem::path& directory,
                                                                      const std::string&           extension)
    {
        std::vector<std::filesystem::path> results;
        std::error_code                    ec;

        if (!std::filesystem::is_directory(directory, ec))
            return results;

        for (const auto& entry : std::filesystem::recursive_directory_iterator(directory, ec))
        {
            if (!entry.is_regular_file())
                continue;
            if (!extension.empty() && entry.path().extension() != extension)
                continue;
            results.push_back(entry.path());
        }
        return results;
    }

    // file operations

    bool FileSystem::createDirectories(const std::filesystem::path& path)
    {
        std::error_code ec;
        std::filesystem::create_directories(path, ec);
        return !ec && std::filesystem::is_directory(path);
    }

    bool FileSystem::copyFile(const std::filesystem::path& source, const std::filesystem::path& destination)
    {
        if (destination.has_parent_path())
            createDirectories(destination.parent_path());

        std::error_code ec;
        std::filesystem::copy_file(source, destination, std::filesystem::copy_options::overwrite_existing, ec);
        if (ec)
        {
            RE_LOG_ERROR("Failed to copy: " + source.string() + " -> " + destination.string());
            return false;
        }
        return true;
    }

    bool FileSystem::removeFile(const std::filesystem::path& path)
    {
        std::error_code ec;
        std::filesystem::remove(path, ec);
        return !ec || !std::filesystem::exists(path);
    }

    std::int64_t FileSystem::getLastWriteTime(const std::filesystem::path& path)
    {
        std::error_code ec;
        auto            ftime = std::filesystem::last_write_time(path, ec);
        if (ec)
            return 0;

        auto duration = ftime.time_since_epoch();
        return std::chrono::duration_cast<std::chrono::seconds>(duration).count();
    }

} // namespace RealmEngine
