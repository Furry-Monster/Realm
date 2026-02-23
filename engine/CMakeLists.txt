set(TARGET_NAME realm)

# Required: platform, core, functional
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
        functional/physics
        functional/animation
        functional/network
        functional/render
        functional/render/rhi
        functional/render/rhi/opengl
        functional/render/ibl
        functional/render/passes
)

# Optional: module (gated by REALM_BUILD_*)
set(ENGINE_MODULES_OPTIONAL "")

if(REALM_BUILD_RENDER)
        list(APPEND ENGINE_MODULES_OPTIONAL
                module/render
                module/render/components
                module/render/components/lighting
        )
endif()
if(REALM_BUILD_AUDIO)
        list(APPEND ENGINE_MODULES_OPTIONAL
                module/audio
                module/audio/components
        )
endif()
if(REALM_BUILD_CAMERA)
        list(APPEND ENGINE_MODULES_OPTIONAL
                module/camera
                module/camera/components
        )
endif()
if(REALM_BUILD_PHYSICS)
        list(APPEND ENGINE_MODULES_OPTIONAL module/physics)
endif()
if(REALM_BUILD_ANIMATION)
        list(APPEND ENGINE_MODULES_OPTIONAL module/animation)
endif()
if(REALM_BUILD_PARTICLES)
        list(APPEND ENGINE_MODULES_OPTIONAL module/particles)
endif()
if(REALM_BUILD_NETWORK)
        list(APPEND ENGINE_MODULES_OPTIONAL module/network)
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

target_link_libraries(${TARGET_NAME}
        PUBLIC Realm::ThirdParty
        PRIVATE $<$<PLATFORM_ID:Windows>:psapi advapi32>
)
