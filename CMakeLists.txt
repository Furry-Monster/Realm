cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

project(RealmEngine
    VERSION 0.1.0
    LANGUAGES CXX C
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable compile commands export for clang-tidy
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(CMakeDependentOption)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra)
endif()

set(OPENGL_GL_PREFERENCE GLVND)

set(REALM_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(CMAKE_INSTALL_PREFIX "${REALM_ROOT_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${REALM_ROOT_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${REALM_ROOT_DIR}/bin/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${REALM_ROOT_DIR}/bin/lib")

add_custom_target(copy_resources ALL
    COMMENT "Copying resource folders to bin directory..."
)

add_custom_command(TARGET copy_resources POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${REALM_ROOT_DIR}/assets"
    "${REALM_ROOT_DIR}/bin/assets"
    COMMENT "Copying assets folder to bin/assets..."
)

add_custom_command(TARGET copy_resources POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${REALM_ROOT_DIR}/shaders"
    "${REALM_ROOT_DIR}/bin/shaders"
    COMMENT "Copying shaders folder to bin/shaders..."
)

add_subdirectory(libs)

add_subdirectory(src)

# Format and Lint targets
find_program(CLANG_FORMAT clang-format)
find_program(CLANG_TIDY clang-tidy)

if(CLANG_FORMAT)
    file(GLOB_RECURSE ALL_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
    )

    add_custom_target(format
        COMMAND ${CLANG_FORMAT} -i ${ALL_SOURCES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Formatting source files with clang-format..."
    )

    add_custom_target(format-check
        COMMAND ${CLANG_FORMAT} --dry-run --Werror ${ALL_SOURCES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Checking source file formatting..."
    )
else()
    message(WARNING "clang-format not found. Format targets will not be available.")
endif()

if(CLANG_TIDY)
    file(GLOB_RECURSE ALL_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
    )

    add_custom_target(lint
        COMMAND ${CLANG_TIDY} ${ALL_SOURCES}
            -p ${CMAKE_BINARY_DIR}
            --format-style=file
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Running clang-tidy on source files..."
    )

    add_custom_target(lint-fix
        COMMAND ${CLANG_TIDY} ${ALL_SOURCES}
            -p ${CMAKE_BINARY_DIR}
            --format-style=file
            --fix
            --fix-errors
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Running clang-tidy with auto-fix on source files..."
    )
else()
    message(WARNING "clang-tidy not found. Lint targets will not be available.")
endif()
