set(TARGET_NAME RealmRuntime)

file(GLOB_RECURSE RUNTIME_SOURCES "*.cpp" "*.c")
file(GLOB_RECURSE RUNTIME_HEADERS "*.hpp" "*.h")

if(RUNTIME_SOURCES)
    add_executable(${TARGET_NAME} ${RUNTIME_SOURCES} ${RUNTIME_HEADERS})

    target_link_libraries(${TARGET_NAME} PRIVATE RealmEngineLib)

    target_include_directories(${TARGET_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
    )
endif()
