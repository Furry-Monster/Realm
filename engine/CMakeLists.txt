set(TARGET_NAME realm)

# Required: platform, core, functional (always built)
set(ENGINE_MODULES_REQUIRED
        platform/filesystem
        platform/info
        platform/input
        platform/window
        core/base
        core/debug
        core/event
        core/log
        core/math
        functional/resource
        functional/scene
        functional/ecs
        functional/ecs/components
        functional/ecs/systems
        functional/render
        functional/render/rhi
        functional/render/rhi/opengl
        functional/render/ibl
        functional/render/passes
        functional/render/components
        functional/render/components/lighting
        functional/camera
        functional/camera/components
        functional/physics
        functional/animation
        functional/particles
)

# Optional modules (gated by REALM_BUILD_*)
set(ENGINE_MODULES_OPTIONAL "")

if(REALM_BUILD_AUDIO)
        list(APPEND ENGINE_MODULES_OPTIONAL
                module/audio
                module/audio/components
        )
endif()
if(REALM_BUILD_NETWORK)
        list(APPEND ENGINE_MODULES_OPTIONAL
                module/network
        )
endif()

set(ENGINE_MODULES ${ENGINE_MODULES_REQUIRED} ${ENGINE_MODULES_OPTIONAL})

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

# Compile definitions for optional modules (used in #if REALM_BUILD_* guards)
target_compile_definitions(${TARGET_NAME} PRIVATE
        $<$<BOOL:${REALM_BUILD_AUDIO}>:REALM_BUILD_AUDIO=1>
        $<$<BOOL:${REALM_BUILD_NETWORK}>:REALM_BUILD_NETWORK=1>
)

target_link_libraries(${TARGET_NAME}
        PUBLIC Realm::ThirdParty
        PRIVATE $<$<PLATFORM_ID:Windows>:psapi advapi32>
)
