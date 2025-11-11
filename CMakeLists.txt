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
