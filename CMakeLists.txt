cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

project(RealmEngine
        VERSION 0.1.0
        LANGUAGES CXX C
        DESCRIPTION "Modern OpenGL game engine with PBR, ECS, and visual editor"
)

# CMake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(RealmOptions)
include(RealmCompiler)
include(RealmUtils)

# Global settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(REALM_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(REALM_BIN_DIR "${REALM_ROOT_DIR}/bin")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${REALM_BIN_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${REALM_BIN_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${REALM_BIN_DIR}/lib")

foreach (_config Debug Release RelWithDebInfo MinSizeRel)
    string(TOUPPER ${_config} _config_upper)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${_config_upper} "${REALM_BIN_DIR}")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${_config_upper} "${REALM_BIN_DIR}/lib")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${_config_upper} "${REALM_BIN_DIR}/lib")
endforeach ()

# Version header
configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/RealmVersion.h.in"
        "${CMAKE_CURRENT_BINARY_DIR}/generated/realm_version.h"
        @ONLY
)

# Resource copying
realm_copy_resources(realm_copy_resources
        DIRECTORIES
        "${REALM_ROOT_DIR}/assets"
        "${REALM_ROOT_DIR}/shaders"
        DESTINATION "${REALM_BIN_DIR}"
)

# Subdirectories
add_subdirectory(libs)
add_subdirectory(engine)

if (REALM_BUILD_EDITOR)
    add_subdirectory(editor)
endif ()

if (REALM_BUILD_SANDBOX)
    add_subdirectory(sandbox)
endif ()

if (REALM_BUILD_TESTS)
    enable_testing()
    # add_subdirectory(tests)
endif ()
