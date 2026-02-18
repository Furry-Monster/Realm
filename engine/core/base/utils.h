#pragma once

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

    inline std::string base64Encode(const std::string& data)
    {
        constexpr char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

        std::string result;
        int         val_a = 0, val_b = -6;
        for (const char c : data)
        {
            val_a = (val_a << 8) + static_cast<unsigned char>(c);
            val_b += 8;
            while (val_b >= 0)
            {
                result.push_back(base64_chars[(val_a >> val_b) & 0x3F]);
                val_b -= 6;
            }
        }
        if (val_b > -6)
            result.push_back(base64_chars[((val_a << 8) >> (val_b + 8)) & 0x3F]);
        while (result.size() % 4)
            result.push_back('=');
        return result;
    }

    inline std::string base64Decode(const std::string& encoded_data)
    {
        constexpr char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

        std::string result;
        int         val = 0, valb = -8;
        for (const char c : encoded_data)
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

    /// <summary>
    /// Extract "ClassName::functionName" from __PRETTY_FUNCTION__ / __FUNCSIG__.
    /// Input  (GCC/Clang): "void RealmEngine::ConfigManager::disposal() const"
    /// Output            : "ConfigManager::disposal"
    /// </summary>
    inline std::string_view extractClassFunction(const char* pretty_function)
    {
        std::string_view pf(pretty_function);

        auto paren = pf.find('(');
        if (paren == std::string_view::npos)
            paren = pf.size();

        const auto space = pf.rfind(' ', paren);
        const auto start = (space == std::string_view::npos) ? static_cast<std::string_view::size_type>(0) : space + 1;
        const std::string_view qualified = pf.substr(start, paren - start);

        const auto last_sep = qualified.rfind("::");
        if (last_sep == std::string_view::npos)
            return qualified;

        const auto before_last = (last_sep >= 2) ? qualified.rfind("::", last_sep - 2) : std::string_view::npos;
        if (before_last == std::string_view::npos)
            return qualified;

        return qualified.substr(before_last + 2);
    }

    // Extract filename from full __FILE__ path
    inline std::string_view extractFileName(const char* path)
    {
        const std::string_view sv(path);
        const auto             pos = sv.find_last_of("/\\");
        return (pos != std::string_view::npos) ? sv.substr(pos + 1) : sv;
    }

} // namespace RealmEngine
