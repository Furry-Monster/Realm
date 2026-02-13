#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace RealmEngine
{
    class FileSystem
    {
    public:
        static std::filesystem::path getExecutablePath() noexcept;
        static std::filesystem::path getExecutableDir() noexcept;

        static std::optional<std::string>          readTextFile(const std::filesystem::path& path);
        static std::optional<std::vector<uint8_t>> readBinaryFile(const std::filesystem::path& path);

        static bool writeTextFile(const std::filesystem::path& path, const std::string& content);
        static bool writeBinaryFile(const std::filesystem::path& path, const std::vector<uint8_t>& data);

        static std::uintmax_t getFileSize(const std::filesystem::path& path);

        static std::vector<std::filesystem::path> listFiles(const std::filesystem::path& directory,
                                                            const std::string&           extension = "");
        static std::vector<std::filesystem::path> listFilesRecursive(const std::filesystem::path& directory,
                                                                     const std::string&           extension = "");

        static bool createDirectories(const std::filesystem::path& path);
        static bool copyFile(const std::filesystem::path& source, const std::filesystem::path& destination);
        static bool removeFile(const std::filesystem::path& path);

        static std::int64_t getLastWriteTime(const std::filesystem::path& path);
    };
} // namespace RealmEngine
