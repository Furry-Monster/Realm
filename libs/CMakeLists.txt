message(STATUS "Building libraries...")

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libs" FORCE)

if(NOT TARGET glfw)
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "..." FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "..." FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "..." FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "..." FORCE)
    add_subdirectory(glfw)
endif()

if(NOT TARGET glad)
    add_library(glad STATIC ${CMAKE_CURRENT_SOURCE_DIR}/glad/src/gl.c)
    target_include_directories(glad PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/glad/include)
endif()

if(NOT TARGET glm)
    add_library(glm INTERFACE)
    target_include_directories(glm INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/glm)
endif()

if(NOT TARGET imgui)
    set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imgui)
    file(GLOB IMGUI_SOURCE
        "${IMGUI_DIR}/*.cpp"
        "${IMGUI_DIR}/backends/imgui_impl_glfw.cpp"
        "${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp"
    )
    add_library(imgui STATIC ${IMGUI_SOURCE})
    target_include_directories(imgui PUBLIC
        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends
    )
    target_link_libraries(imgui PUBLIC glfw)
    target_compile_definitions(imgui PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLAD)
endif()

if(NOT TARGET stb)
    add_library(stb INTERFACE)
    target_include_directories(stb INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/stb)
endif()

if(NOT TARGET assimp)
    set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "..." FORCE)
    set(ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "..." FORCE)
    set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "..." FORCE)
    set(ASSIMP_INSTALL OFF CACHE BOOL "..." FORCE)
    set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "..." FORCE)
    set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "..." FORCE)
    set(ASSIMP_BUILD_OBJ_IMPORTER ON CACHE BOOL "..." FORCE)
    set(ASSIMP_BUILD_FBX_IMPORTER ON CACHE BOOL "..." FORCE)
    set(ASSIMP_BUILD_GLTF_IMPORTER ON CACHE BOOL "..." FORCE)
    set(ASSIMP_BUILD_PLY_IMPORTER ON CACHE BOOL "..." FORCE)
    set(ASSIMP_BUILD_STL_IMPORTER ON CACHE BOOL "..." FORCE)
    add_subdirectory(assimp)
endif()

if(NOT TARGET spdlog)
    set(SPDLOG_BUILD_EXAMPLE OFF CACHE BOOL "..." FORCE)
    set(SPDLOG_BUILD_TESTS OFF CACHE BOOL "..." FORCE)
    set(SPDLOG_BUILD_BENCH OFF CACHE BOOL "..." FORCE)
    set(SPDLOG_INSTALL OFF CACHE BOOL "..." FORCE)
    add_subdirectory(spdlog)
endif()

add_library(reflibs INTERFACE)
target_link_libraries(reflibs INTERFACE glad glfw glm imgui assimp spdlog)

if(UNIX AND NOT APPLE)
    find_package(OpenGL REQUIRED)
    find_package(X11 REQUIRED)
    target_link_libraries(reflibs INTERFACE
        OpenGL::GL
        ${X11_LIBRARIES}
    )
endif()

target_include_directories(reflibs INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/glfw/include
    ${CMAKE_CURRENT_SOURCE_DIR}/glad/include
    ${CMAKE_CURRENT_SOURCE_DIR}/glm
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/stb
    ${CMAKE_CURRENT_SOURCE_DIR}/assimp/include
    ${CMAKE_CURRENT_SOURCE_DIR}/spdlog/include
)
