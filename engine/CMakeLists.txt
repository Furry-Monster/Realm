set(TARGET_NAME realm)

# Required: platform, core, functional (always compiled)
set(ENGINE_MODULES
        core/base
        core/debug
        core/event
        core/log
        core/math
        platform/filesystem
        platform/info
        platform/input
        platform/window
        functional/resource
        functional/scene
        functional/physics
        functional/animation
        functional/network
        module/render/rhi
        module/render/rhi/opengl
        module/render
        module/render/ibl
        module/render/passes
        module/ecs
        module/ecs/components
        module/ecs/components/lighting
        module/ecs/components/audio
        module/ecs/systems
)

if (REALM_BUILD_PHYSICS)
    list(APPEND ENGINE_MODULES module/physics)
endif ()
if (REALM_BUILD_ANIMATION)
    list(APPEND ENGINE_MODULES module/animation)
endif ()
if (REALM_BUILD_PARTICLES)
    list(APPEND ENGINE_MODULES module/particles)
endif ()
if (REALM_BUILD_NETWORK)
    list(APPEND ENGINE_MODULES module/network)
endif ()
if (REALM_BUILD_GAME)
    list(APPEND ENGINE_MODULES module/game)
endif ()

file(GLOB ENGINE_ROOT_FILES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
)

realm_collect_sources(ENGINE_SOURCES
        DIRECTORIES ${ENGINE_MODULES}
        ROOT_FILES ${ENGINE_ROOT_FILES}
)

add_library(${TARGET_NAME} STATIC ${ENGINE_SOURCES})
add_library(Realm::Engine ALIAS ${TARGET_NAME})

realm_target_common_setup(${TARGET_NAME} FOLDER "Engine")

target_include_directories(${TARGET_NAME}
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
        PRIVATE ${CMAKE_BINARY_DIR}/generated
)

target_link_libraries(${TARGET_NAME}
        PUBLIC Realm::ThirdParty
        PRIVATE $<$<PLATFORM_ID:Windows>:psapi advapi32>
)
