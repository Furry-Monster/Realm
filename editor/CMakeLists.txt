set(TARGET_NAME RealmEditor)

file(GLOB_RECURSE EDITOR_SOURCES "*.cpp" "*.c")
file(GLOB_RECURSE EDITOR_HEADERS "*.hpp" "*.h")

if(EDITOR_SOURCES)
    add_executable(${TARGET_NAME} ${EDITOR_SOURCES} ${EDITOR_HEADERS})

    target_link_libraries(${TARGET_NAME} PRIVATE RealmEngineLib)

    target_include_directories(${TARGET_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
    )
endif()
