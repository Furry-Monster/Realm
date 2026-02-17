#pragma once

#include <cstdint>
#include <cstring>
#include <string>

namespace RealmEngine
{
    inline constexpr const char* DEFAULT_ENCRYPTION_KEY = "Elysia";

    inline std::string xorEncrypt(const std::string& data, const std::string& key)
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

    inline std::string xorDecrypt(const std::string& encrypted_data, const std::string& key)
    {
        return xorEncrypt(encrypted_data, key);
    }

    inline constexpr uint32_t FNV_OFFSET_BASIS = 2166136261u;
    inline constexpr uint32_t FNV_PRIME        = 16777619u;

    inline uint32_t hashString(const std::string& str)
    {
        uint32_t hash = FNV_OFFSET_BASIS;
        for (char c : str)
        {
            hash ^= static_cast<uint32_t>(c);
            hash *= FNV_PRIME;
        }
        return hash;
    }

    inline constexpr uint64_t FNV_OFFSET_BASIS_64 = 14695981039346656037ull;
    inline constexpr uint64_t FNV_PRIME_64        = 1099511628211ull;

    inline uint64_t hashString64(const std::string& str)
    {
        uint64_t hash = FNV_OFFSET_BASIS_64;
        for (char c : str)
        {
            hash ^= static_cast<uint64_t>(c);
            hash *= FNV_PRIME_64;
        }
        return hash;
    }

    inline std::string base64Encode(const std::string& data)
    {
        const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

        std::string result;
        int         val = 0, valb = -6;
        for (char c : data)
        {
            val = (val << 8) + static_cast<unsigned char>(c);
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

    inline std::string base64Decode(const std::string& encoded_data)
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

            val = (val << 6) + static_cast<int>(pos - base64_chars);
            valb += 6;
            if (valb >= 0)
            {
                result.push_back(static_cast<char>((val >> valb) & 0xFF));
                valb -= 8;
            }
        }
        return result;
    }

    inline std::string obfuscateString(const std::string& str, uint32_t seed = 0x12345678)
    {
        std::string result;
        result.reserve(str.size());
        for (char i : str)
        {
            seed = seed * 1103515245 + 12345;
            result += static_cast<char>(static_cast<unsigned char>(i) ^ (seed & 0xFF));
        }
        return result;
    }

    // XOR-based obfuscation is self-inverse
    inline std::string deobfuscateString(const std::string& obfuscated, uint32_t seed = 0x12345678)
    {
        return obfuscateString(obfuscated, seed);
    }

} // namespace RealmEngine
