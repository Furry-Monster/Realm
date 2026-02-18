# Collect sources from directories with IDE source groups
function(realm_collect_sources output_var)
    cmake_parse_arguments(ARG "" "" "DIRECTORIES;ROOT_FILES" ${ARGN})

    set(_all_sources "")

    if (ARG_ROOT_FILES)
        list(APPEND _all_sources ${ARG_ROOT_FILES})
        source_group("" FILES ${ARG_ROOT_FILES})
    endif ()

    foreach (_dir ${ARG_DIRECTORIES})
        file(GLOB _dir_sources CONFIGURE_DEPENDS
                "${CMAKE_CURRENT_SOURCE_DIR}/${_dir}/*.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/${_dir}/*.c"
                "${CMAKE_CURRENT_SOURCE_DIR}/${_dir}/*.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/${_dir}/*.hpp"
        )
        if (_dir_sources)
            string(REPLACE "/" "\\" _group "${_dir}")
            source_group("${_group}" FILES ${_dir_sources})
            list(APPEND _all_sources ${_dir_sources})
        endif ()
    endforeach ()

    set(${output_var} ${_all_sources} PARENT_SCOPE)
endfunction()

# Copy resource directories to destination
function(realm_copy_resources target_name)
    cmake_parse_arguments(ARG "" "DESTINATION" "DIRECTORIES" ${ARGN})

    add_custom_target(${target_name} ALL)

    foreach (_dir ${ARG_DIRECTORIES})
        get_filename_component(_dirname "${_dir}" NAME)
        add_custom_command(TARGET ${target_name} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory "${_dir}" "${ARG_DESTINATION}/${_dirname}"
        )
    endforeach ()
endfunction()

# Apply common settings to a target
function(realm_target_common_setup target)
    cmake_parse_arguments(ARG "" "FOLDER" "" ${ARGN})

    target_link_libraries(${target} PRIVATE
            Realm::CompilerOptions
            Realm::SanitizerOptions
    )

    if (ARG_FOLDER)
        set_target_properties(${target} PROPERTIES FOLDER "${ARG_FOLDER}")
    endif ()
endfunction()
