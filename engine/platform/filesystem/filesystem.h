#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace RealmEngine
{
    /// Utility class for common file I/O operations.
    /// Wraps std::filesystem with convenient helpers for engine-level file access.
    class FileSystem
    {
    public:
        /// Read an entire text file into a string.
        /// Returns std::nullopt if the file cannot be opened or read.
        static std::optional<std::string> readTextFile(const std::filesystem::path& path);

        /// Read an entire binary file into a byte vector.
        /// Returns std::nullopt if the file cannot be opened or read.
        static std::optional<std::vector<uint8_t>> readBinaryFile(const std::filesystem::path& path);

        /// Write a string to a text file. Creates parent directories if needed.
        /// Returns true on success.
        static bool writeTextFile(const std::filesystem::path& path, const std::string& content);

        /// Write a byte vector to a binary file. Creates parent directories if needed.
        /// Returns true on success.
        static bool writeBinaryFile(const std::filesystem::path& path, const std::vector<uint8_t>& data);

        /// Get the size of a file in bytes. Returns 0 if the file does not exist.
        static std::uintmax_t getFileSize(const std::filesystem::path& path);

        /// List files in a directory (non-recursive).
        /// If extension is not empty, only files with that extension are returned.
        /// Extension should include the dot, e.g. ".json".
        static std::vector<std::filesystem::path> listFiles(const std::filesystem::path& directory,
                                                            const std::string&           extension = "");

        /// List files in a directory recursively.
        /// If extension is not empty, only files with that extension are returned.
        static std::vector<std::filesystem::path> listFilesRecursive(const std::filesystem::path& directory,
                                                                     const std::string&           extension = "");

        /// Create directories (including parent directories) if they don't exist.
        /// Returns true if the directories exist after the call.
        static bool createDirectories(const std::filesystem::path& path);

        /// Copy a file, overwriting the destination if it exists.
        /// Returns true on success.
        static bool copyFile(const std::filesystem::path& source, const std::filesystem::path& destination);

        /// Remove a single file.
        /// Returns true if the file was removed or did not exist.
        static bool removeFile(const std::filesystem::path& path);

        /// Get last write time as a platform-consistent timestamp (seconds).
        /// Values are only meaningful for comparison within the same platform and session.
        static std::int64_t getLastWriteTime(const std::filesystem::path& path);
    };
} // namespace RealmEngine
