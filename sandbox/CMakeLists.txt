set(TARGET_NAME RealmRuntime)

file(GLOB RUNTIME_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
)

add_executable(${TARGET_NAME} ${RUNTIME_SOURCES})

realm_target_common_setup(${TARGET_NAME} FOLDER "Runtime")

target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${TARGET_NAME} PRIVATE Realm::Engine)
target_compile_definitions(${TARGET_NAME} PRIVATE REALM_RUNTIME)

add_dependencies(${TARGET_NAME} realm_copy_resources)
