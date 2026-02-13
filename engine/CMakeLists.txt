set(TARGET_NAME RealmEngineLib)

file(GLOB_RECURSE ENGINE_SOURCES "*.cpp" "*.c")
file(GLOB_RECURSE ENGINE_HEADERS "*.hpp" "*.h")

if(ENGINE_SOURCES)
    add_library(${TARGET_NAME} STATIC ${ENGINE_SOURCES} ${ENGINE_HEADERS})

    target_link_libraries(${TARGET_NAME} PUBLIC reflibs)

    target_include_directories(${TARGET_NAME} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    )
endif()
