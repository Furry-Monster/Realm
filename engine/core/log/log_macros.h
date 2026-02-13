#pragma once

#include <string>

#include "core/base/macros.h"
#include "core/log/logger.h"

// Logging convenience macros.
//
// Log output format:
//   Default:  [info] [ConfigManager::disposal] Config saved to: ...
//   Verbose:  [info] [ConfigManager::disposal @ config_manager.cpp:37] Config saved to: ...
//
// Compile-time controls (define before including, or via CMakeLists.txt):
//   RE_MIN_LOG_LEVEL  - Minimum log level (0=debug, 1=info, 2=warn, 3=error, 4=fatal)
//   RE_LOG_VERBOSE    - Include source file and line number in output

#ifndef RE_MIN_LOG_LEVEL
#  if defined(NDEBUG) || defined(_NDEBUG)
#    define RE_MIN_LOG_LEVEL 1
#  else
#    define RE_MIN_LOG_LEVEL 0
#  endif
#endif

#ifdef RE_LOG_VERBOSE
#  define RE_LOG_SOURCE_LOC " @ " + std::string(RealmEngine::extractFileName(__FILE__)) + ":" + std::to_string(__LINE__)
#else
#  define RE_LOG_SOURCE_LOC ""
#endif

#define RE_LOG_IMPL(level_value, level_enum, msg)                                                                      \
    do                                                                                                                 \
    {                                                                                                                  \
        if constexpr ((level_value) >= RE_MIN_LOG_LEVEL)                                                               \
        {                                                                                                              \
            if (RealmEngine::g_logger)                                                                                 \
                RealmEngine::g_logger->log(level_enum,                                                                 \
                                           "[" + std::string(RealmEngine::extractClassFunction(RE_PRETTY_FUNCTION)) +  \
                                               RE_LOG_SOURCE_LOC + "] " + (msg));                                      \
        }                                                                                                              \
    } while (0)

#define RE_LOG_DEBUG(msg) RE_LOG_IMPL(0, RealmEngine::Logger::LogLevel::debug, msg)
#define RE_LOG_INFO(msg) RE_LOG_IMPL(1, RealmEngine::Logger::LogLevel::info, msg)
#define RE_LOG_WARN(msg) RE_LOG_IMPL(2, RealmEngine::Logger::LogLevel::warn, msg)
#define RE_LOG_ERROR(msg) RE_LOG_IMPL(3, RealmEngine::Logger::LogLevel::error, msg)
#define RE_LOG_FATAL(msg) RE_LOG_IMPL(4, RealmEngine::Logger::LogLevel::fatal, msg)
