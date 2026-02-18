set(TARGET_NAME RealmEditor)

set(EDITOR_MODULES
        bridge
        commands
        hotkey
        panels
        preferences
)

file(GLOB EDITOR_ROOT_FILES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
)

realm_collect_sources(EDITOR_SOURCES
        DIRECTORIES ${EDITOR_MODULES}
        ROOT_FILES ${EDITOR_ROOT_FILES}
)

add_executable(${TARGET_NAME} ${EDITOR_SOURCES})

realm_target_common_setup(${TARGET_NAME} FOLDER "Editor")

target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${TARGET_NAME} PRIVATE Realm::Engine)
target_compile_definitions(${TARGET_NAME} PRIVATE REALM_EDITOR)

add_dependencies(${TARGET_NAME} realm_copy_resources)
