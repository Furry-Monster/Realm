#pragma once

#include <string_view>

// Cross-platform pretty function macro
#if defined(__GNUC__) || defined(__clang__)
#  define RE_PRETTY_FUNCTION __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
#  define RE_PRETTY_FUNCTION __FUNCSIG__
#else
#  define RE_PRETTY_FUNCTION __func__
#endif

namespace RealmEngine
{
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

        auto space = pf.rfind(' ', paren);
        auto start = (space == std::string_view::npos) ? std::string_view::size_type(0) : space + 1;

        auto qualified = pf.substr(start, paren - start);

        auto last_sep = qualified.rfind("::");
        if (last_sep == std::string_view::npos)
            return qualified;

        auto before_last = (last_sep >= 2) ? qualified.rfind("::", last_sep - 2) : std::string_view::npos;
        if (before_last == std::string_view::npos)
            return qualified;

        return qualified.substr(before_last + 2);
    }

    // Extract filename from full __FILE__ path
    inline std::string_view extractFileName(const char* path)
    {
        std::string_view sv(path);
        auto             pos = sv.find_last_of("/\\");
        return (pos != std::string_view::npos) ? sv.substr(pos + 1) : sv;
    }

} // namespace RealmEngine
