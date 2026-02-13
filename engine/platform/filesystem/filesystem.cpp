#include "platform/filesystem/filesystem.h"

#include "core/log/log_macros.h"

#include <chrono>
#include <fstream>

namespace RealmEngine
{
    std::optional<std::string> FileSystem::readTextFile(const std::filesystem::path& path)
    {
        std::ifstream file(path, std::ios::in);
        if (!file.is_open())
        {
            RE_LOG_ERROR("Failed to open text file for reading: " + path.string());
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
            RE_LOG_ERROR("Failed to open binary file for reading: " + path.string());
            return std::nullopt;
        }

        auto size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<uint8_t> data(static_cast<size_t>(size));
        if (!file.read(reinterpret_cast<char*>(data.data()), size))
        {
            RE_LOG_ERROR("Failed to read binary file: " + path.string());
            return std::nullopt;
        }
        return data;
    }

    bool FileSystem::writeTextFile(const std::filesystem::path& path, const std::string& content)
    {
        if (path.has_parent_path())
            createDirectories(path.parent_path());

        std::ofstream file(path, std::ios::out | std::ios::trunc);
        if (!file.is_open())
        {
            RE_LOG_ERROR("Failed to open text file for writing: " + path.string());
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
            RE_LOG_ERROR("Failed to open binary file for writing: " + path.string());
            return false;
        }

        file.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
        return file.good();
    }

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
            RE_LOG_ERROR("Failed to copy file: " + source.string() + " -> " + destination.string());
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
