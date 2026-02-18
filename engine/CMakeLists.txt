set(TARGET_NAME realm)

# Module directories
set(ENGINE_MODULES
        core/base
        core/debug
        core/event
        core/geometry
        core/log
        core/math
        platform/filesystem
        platform/info
        platform/input
        platform/window
        render/rhi
        render/rhi/opengl
        render
        render/ibl
        render/passes
        resource
        scene
        scene/components
        scene/components/lighting
        scene/serialization
        scene/systems
)

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
