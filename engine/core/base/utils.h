#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include <string_view>

#include "global_context.h"
#include "core/log/logger.h"

namespace RealmEngine
{
    // Encryption helpers
    static constexpr const char* DEFAULT_ENCRYPTION_KEY = "Elysia";

    inline static std::string xorEncrypt(const std::string& data, const std::string& key)
    {
        if (key.empty())
            return data;

        std::string result;
        result.reserve(data.size());
        for (size_t i = 0; i < data.size(); ++i)
        {
            result += static_cast<char>(data[i] ^ key[i % key.size()]);
        }
        return result;
    }

    inline static std::string xorDecrypt(const std::string& encrypted_data, const std::string& key)
    {
        return xorEncrypt(encrypted_data, key);
    }

    static constexpr uint32_t FNV_OFFSET_BASIS = 2166136261u;
    static constexpr uint32_t FNV_PRIME        = 16777619u;

    inline static uint32_t hashString(const std::string& str)
    {
        uint32_t hash = FNV_OFFSET_BASIS;
        for (char c : str)
        {
            hash ^= static_cast<uint32_t>(c);
            hash *= FNV_PRIME;
        }
        return hash;
    }

    static constexpr uint64_t FNV_OFFSET_BASIS_64 = 14695981039346656037ull;
    static constexpr uint64_t FNV_PRIME_64        = 1099511628211ull;

    inline static uint64_t hashString64(const std::string& str)
    {
        uint64_t hash = FNV_OFFSET_BASIS_64;
        for (char c : str)
        {
            hash ^= static_cast<uint64_t>(c);
            hash *= FNV_PRIME_64;
        }
        return hash;
    }

    inline static std::string base64Encode(const std::string& data)
    {
        const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

        std::string result;
        int         val = 0, valb = -6;
        for (unsigned char c : data)
        {
            val = (val << 8) + c;
            valb += 8;
            while (valb >= 0)
            {
                result.push_back(base64_chars[(val >> valb) & 0x3F]);
                valb -= 6;
            }
        }
        if (valb > -6)
            result.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
        while (result.size() % 4)
            result.push_back('=');
        return result;
    }

    inline static std::string base64Decode(const std::string& encoded_data)
    {
        const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

        std::string result;
        int         val = 0, valb = -8;
        for (char c : encoded_data)
        {
            if (c == '=')
                break;

            const char* pos = std::strchr(base64_chars, c);
            if (pos == nullptr)
                continue;

            val = (val << 6) + (pos - base64_chars);
            valb += 6;
            if (valb >= 0)
            {
                result.push_back(static_cast<char>((val >> valb) & 0xFF));
                valb -= 8;
            }
        }
        return result;
    }

    inline static std::string obfuscateString(const std::string& str, uint32_t seed = 0x12345678)
    {
        std::string result;
        result.reserve(str.size());
        for (char i : str)
        {
            seed = seed * 1103515245 + 12345;
            result += static_cast<char>(i ^ (seed & 0xFF));
        }
        return result;
    }

    inline static std::string deobfuscateString(const std::string& obfuscated, uint32_t seed = 0x12345678)
    {
        std::string result;
        result.reserve(obfuscated.size());
        for (char i : obfuscated)
        {
            seed = seed * 1103515245 + 12345;
            result += static_cast<char>(i ^ (seed & 0xFF));
        }
        return result;
    }

    // =====================================================================================
    // Logger helpers
    // =====================================================================================
    //
    // Log output format:
    //   Default:  [info] [ConfigManager::disposal] Config saved to: ...
    //   Verbose:  [info] [ConfigManager::disposal @ config_manager.cpp:37] Config saved to: ...
    //
    // Compile-time controls (define before including this header, or via CMakeLists.txt):
    //   RE_MIN_LOG_LEVEL  - Minimum log level (0=debug, 1=info, 2=warn, 3=error, 4=fatal)
    //   RE_LOG_VERBOSE    - Include source file and line number in log output
    //
    // Example usage in CMakeLists.txt:
    //   target_compile_definitions(MyTarget PRIVATE RE_MIN_LOG_LEVEL=2)    # Only WARN and above
    //   target_compile_definitions(MyTarget PRIVATE RE_LOG_VERBOSE)        # Enable verbose logging

    // -------------------------------------------------------------------------------------
    // Cross-platform pretty function macro
    // -------------------------------------------------------------------------------------
#if defined(__GNUC__) || defined(__clang__)
#  define RE_PRETTY_FUNCTION __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
#  define RE_PRETTY_FUNCTION __FUNCSIG__
#else
#  define RE_PRETTY_FUNCTION __func__
#endif

    // -------------------------------------------------------------------------------------
    // Helper: extract "ClassName::functionName" from __PRETTY_FUNCTION__ / __FUNCSIG__
    //
    //   Input  (GCC/Clang): "void RealmEngine::ConfigManager::disposal() const"
    //   Output            : "ConfigManager::disposal"
    //
    //   Input  (free func): "void RealmEngine::someFunction(int)"
    //   Output            : "someFunction"
    // -------------------------------------------------------------------------------------
    inline std::string_view extractClassFunction(const char* pretty_function)
    {
        std::string_view pf(pretty_function);

        // Find first '(' — marks end of the qualified function name
        auto paren = pf.find('(');
        if (paren == std::string_view::npos)
            paren = pf.size();

        // Find last space before '(' — marks start of the qualified name (skipping return type)
        auto space = pf.rfind(' ', paren);
        auto start = (space == std::string_view::npos) ? std::string_view::size_type(0) : space + 1;

        auto qualified = pf.substr(start, paren - start);

        // Extract last two "::"-separated components: "Class::function"
        auto last_sep = qualified.rfind("::");
        if (last_sep == std::string_view::npos)
            return qualified; // Free function — no class

        // Look for the "::" before the last one to locate the class boundary
        auto before_last = (last_sep >= 2) ? qualified.rfind("::", last_sep - 2) : std::string_view::npos;
        if (before_last == std::string_view::npos)
            return qualified; // Already in "Class::function" form

        return qualified.substr(before_last + 2);
    }

    // -------------------------------------------------------------------------------------
    // Helper: extract just the filename from a full __FILE__ path
    //
    //   Input : "/home/user/project/src/resource/config_manager.cpp"
    //   Output: "config_manager.cpp"
    // -------------------------------------------------------------------------------------
    inline std::string_view extractFileName(const char* path)
    {
        std::string_view sv(path);
        auto             pos = sv.find_last_of("/\\");
        return (pos != std::string_view::npos) ? sv.substr(pos + 1) : sv;
    }

    // -------------------------------------------------------------------------------------
    // Log level gate
    // -------------------------------------------------------------------------------------
#ifndef RE_MIN_LOG_LEVEL
#  if defined(NDEBUG) || defined(_NDEBUG)
// Release build: exclude DEBUG logs by default
#    define RE_MIN_LOG_LEVEL 1
#  else
// Debug build: include all logs
#    define RE_MIN_LOG_LEVEL 0
#  endif
#endif

    // -------------------------------------------------------------------------------------
    // Source location suffix (enabled by RE_LOG_VERBOSE)
    // -------------------------------------------------------------------------------------
#ifdef RE_LOG_VERBOSE
#  define RE_LOG_SOURCE_LOC " @ " + std::string(RealmEngine::extractFileName(__FILE__)) + ":" + std::to_string(__LINE__)
#else
#  define RE_LOG_SOURCE_LOC ""
#endif

    // -------------------------------------------------------------------------------------
    // Core logging macro
    // -------------------------------------------------------------------------------------
#define RE_LOG_IMPL(level_value, level_enum, msg)                                                                      \
    do                                                                                                                 \
    {                                                                                                                  \
        if constexpr ((level_value) >= RE_MIN_LOG_LEVEL)                                                               \
        {                                                                                                              \
            RealmEngine::g_context.m_logger->log(                                                                      \
                level_enum,                                                                                            \
                "[" + std::string(RealmEngine::extractClassFunction(RE_PRETTY_FUNCTION)) + RE_LOG_SOURCE_LOC + "] " +  \
                    (msg));                                                                                            \
        }                                                                                                              \
    } while (0)

#define RE_LOG_DEBUG(msg) RE_LOG_IMPL(0, RealmEngine::Logger::LogLevel::debug, msg)
#define RE_LOG_INFO(msg) RE_LOG_IMPL(1, RealmEngine::Logger::LogLevel::info, msg)
#define RE_LOG_WARN(msg) RE_LOG_IMPL(2, RealmEngine::Logger::LogLevel::warn, msg)
#define RE_LOG_ERROR(msg) RE_LOG_IMPL(3, RealmEngine::Logger::LogLevel::error, msg)
#define RE_LOG_FATAL(msg) RE_LOG_IMPL(4, RealmEngine::Logger::LogLevel::fatal, msg)

} // namespace RealmEngine
