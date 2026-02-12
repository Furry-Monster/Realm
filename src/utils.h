#pragma once

#include <cstdint>
#include <cstring>
#include <string>

#include "global_context.h"
#include "logger.h"

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

    // Logger helpers

    inline static void info(const std::string& str)
    {
        g_context.m_logger->log(Logger::LogLevel::info, "[" + std::string(__FUNCTION__) + "]" + str);
    }

    inline static void info(std::string&& str)
    {
        g_context.m_logger->log(Logger::LogLevel::info, "[" + std::string(__FUNCTION__) + "]" + str);
    }

    inline static void debug(const std::string& str)
    {
        g_context.m_logger->log(Logger::LogLevel::debug, "[" + std::string(__FUNCTION__) + "]" + str);
    }

    inline static void debug(std::string&& str)
    {
        g_context.m_logger->log(Logger::LogLevel::debug, "[" + std::string(__FUNCTION__) + "]" + str);
    }

    inline static void warn(const std::string& str)
    {
        g_context.m_logger->log(Logger::LogLevel::warn, "[" + std::string(__FUNCTION__) + "]" + str);
    }

    inline static void warn(std::string&& str)
    {
        g_context.m_logger->log(Logger::LogLevel::warn, "[" + std::string(__FUNCTION__) + "]" + str);
    }

    inline static void err(const std::string& str)
    {
        g_context.m_logger->log(Logger::LogLevel::error, "[" + std::string(__FUNCTION__) + "]" + str);
    }

    inline static void err(std::string&& str)
    {
        g_context.m_logger->log(Logger::LogLevel::error, "[" + std::string(__FUNCTION__) + "]" + str);
    }

    inline static void fatal(const std::string& str)
    {
        g_context.m_logger->log(Logger::LogLevel::fatal, "[" + std::string(__FUNCTION__) + "]" + str);
    }

    inline static void fatal(std::string&& str)
    {
        g_context.m_logger->log(Logger::LogLevel::fatal, "[" + std::string(__FUNCTION__) + "]" + str);
    }

} // namespace RealmEngine
