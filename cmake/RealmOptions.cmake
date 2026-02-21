include(CMakeDependentOption)

# Build targets
option(REALM_BUILD_EDITOR "Build the visual editor" ON)
option(REALM_BUILD_SANDBOX "Build the standalone sandbox" ON)
option(REALM_BUILD_TESTS "Build unit tests" OFF)

# Features
option(REALM_ENABLE_LOGGING "Enable spdlog-based logging" ON)
option(REALM_ENABLE_PROFILER "Enable profiler" ON)

# Developer tools
option(REALM_ENABLE_SANITIZERS "Enable Address/UB sanitizers" OFF)
option(REALM_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)

# Default build type
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
            "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
endif ()

# Summary
message(STATUS "")
message(STATUS "====== RealmEngine Build Configuration ======")
message(STATUS "  Build type        : ${CMAKE_BUILD_TYPE}")
message(STATUS "  Editor            : ${REALM_BUILD_EDITOR}")
message(STATUS "  Sandbox           : ${REALM_BUILD_SANDBOX}")
message(STATUS "  Tests             : ${REALM_BUILD_TESTS}")
message(STATUS "  Logging           : ${REALM_ENABLE_LOGGING}")
message(STATUS "  Profiler          : ${REALM_ENABLE_PROFILER}")
message(STATUS "  Sanitizers        : ${REALM_ENABLE_SANITIZERS}")
message(STATUS "  Warnings as errors: ${REALM_WARNINGS_AS_ERRORS}")
message(STATUS "==============================================")
message(STATUS "")
