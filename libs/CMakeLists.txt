message(STATUS "[Realm] Configuring third-party libraries ...")

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# GLFW
if (NOT TARGET glfw)
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
    add_subdirectory(glfw)
    get_target_property(_glfw_inc glfw INTERFACE_INCLUDE_DIRECTORIES)
    if (_glfw_inc)
        set_target_properties(glfw PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${_glfw_inc}")
    endif ()
endif ()

# GLAD
if (NOT TARGET glad)
    add_library(glad STATIC "${CMAKE_CURRENT_SOURCE_DIR}/glad/src/glad.c")
    target_include_directories(glad SYSTEM PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/glad/include")
endif ()

# GLM
if (NOT TARGET glm)
    add_library(glm INTERFACE)
    target_include_directories(glm SYSTEM INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/glm")
endif ()

# ImGui
if (NOT TARGET imgui)
    set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/imgui")
    file(GLOB IMGUI_CORE_SOURCES "${IMGUI_DIR}/*.cpp")
    add_library(imgui STATIC
            ${IMGUI_CORE_SOURCES}
            "${IMGUI_DIR}/backends/imgui_impl_glfw.cpp"
            "${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp"
    )
    target_include_directories(imgui SYSTEM PUBLIC "${IMGUI_DIR}" "${IMGUI_DIR}/backends")
    target_link_libraries(imgui PUBLIC glfw)
    target_compile_definitions(imgui PUBLIC IMGUI_HAS_DOCK)
endif ()

# ImGuizmo
if (NOT TARGET imguizmo)
    add_library(imguizmo STATIC "${CMAKE_CURRENT_SOURCE_DIR}/ImGuizmo/ImGuizmo.cpp")
    target_include_directories(imguizmo SYSTEM PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/ImGuizmo")
    target_link_libraries(imguizmo PUBLIC imgui)
endif ()

# stb
if (NOT TARGET stb)
    add_library(stb INTERFACE)
    target_include_directories(stb SYSTEM INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/stb")
endif ()

# Assimp
if (NOT TARGET assimp)
    set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
    set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
    set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
    set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)
    set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "" FORCE)
    set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
    set(ASSIMP_BUILD_OBJ_IMPORTER ON CACHE BOOL "" FORCE)
    set(ASSIMP_BUILD_FBX_IMPORTER ON CACHE BOOL "" FORCE)
    set(ASSIMP_BUILD_GLTF_IMPORTER ON CACHE BOOL "" FORCE)
    set(ASSIMP_BUILD_PLY_IMPORTER ON CACHE BOOL "" FORCE)
    set(ASSIMP_BUILD_STL_IMPORTER ON CACHE BOOL "" FORCE)
    add_subdirectory(assimp)
endif ()

# miniaudio
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/miniaudio/CMakeLists.txt")
    set(MINIAUDIO_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(MINIAUDIO_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(MINIAUDIO_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
    set(MINIAUDIO_INSTALL OFF CACHE BOOL "" FORCE)
    add_subdirectory(miniaudio)
endif ()

# spdlog
if (NOT TARGET spdlog)
    set(SPDLOG_BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)
    set(SPDLOG_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(SPDLOG_BUILD_BENCH OFF CACHE BOOL "" FORCE)
    set(SPDLOG_INSTALL OFF CACHE BOOL "" FORCE)
    add_subdirectory(spdlog)
endif ()

# nlohmann/json
if (NOT TARGET nlohmann)
    add_library(nlohmann INTERFACE)
    target_include_directories(nlohmann SYSTEM INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/nlohmann")
endif ()

# EnTT
if (NOT TARGET entt)
    add_library(entt INTERFACE)
    target_include_directories(entt SYSTEM INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/entt/src")
endif ()

# OpenGL (platform-specific)
add_library(realm_opengl INTERFACE)
if (UNIX AND NOT APPLE)
    set(OpenGL_GL_PREFERENCE GLVND)
    find_package(OpenGL REQUIRED)
    find_package(X11 REQUIRED)
    target_link_libraries(realm_opengl INTERFACE OpenGL::GL ${X11_LIBRARIES})
elseif (WIN32)
    target_link_libraries(realm_opengl INTERFACE opengl32)
elseif (APPLE)
    find_package(OpenGL REQUIRED)
    target_link_libraries(realm_opengl INTERFACE
            OpenGL::GL "-framework Cocoa" "-framework IOKit" "-framework CoreVideo")
endif ()

# Aggregate target
add_library(realm_third_party INTERFACE)
add_library(Realm::ThirdParty ALIAS realm_third_party)
set(REALM_THIRD_PARTY_LIBS glad glfw glm imgui imguizmo stb assimp spdlog nlohmann entt realm_opengl)
if (TARGET miniaudio)
    list(APPEND REALM_THIRD_PARTY_LIBS miniaudio)
endif ()
target_link_libraries(realm_third_party INTERFACE ${REALM_THIRD_PARTY_LIBS})
