# Compiler options target
add_library(realm_compiler_options INTERFACE)
add_library(Realm::CompilerOptions ALIAS realm_compiler_options)

target_compile_features(realm_compiler_options INTERFACE cxx_std_17)

if(MSVC)
    target_compile_options(realm_compiler_options INTERFACE
        /W4 /permissive- /utf-8
        /external:anglebrackets /external:W0
    )
    if(REALM_WARNINGS_AS_ERRORS)
        target_compile_options(realm_compiler_options INTERFACE /WX)
    endif()
else()
    target_compile_options(realm_compiler_options INTERFACE
        -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Wunused -Wnon-virtual-dtor
    )
    if(REALM_WARNINGS_AS_ERRORS)
        target_compile_options(realm_compiler_options INTERFACE -Werror)
    endif()
endif()

# Platform definitions
if(WIN32)
    target_compile_definitions(realm_compiler_options INTERFACE
        REALM_PLATFORM_WINDOWS NOMINMAX _CRT_SECURE_NO_WARNINGS)
elseif(APPLE)
    target_compile_definitions(realm_compiler_options INTERFACE REALM_PLATFORM_MACOS)
else()
    target_compile_definitions(realm_compiler_options INTERFACE REALM_PLATFORM_LINUX)
endif()

target_compile_definitions(realm_compiler_options INTERFACE
    $<$<CONFIG:Debug>:REALM_DEBUG>
    $<$<CONFIG:Release>:REALM_RELEASE>
    $<$<CONFIG:RelWithDebInfo>:REALM_RELEASE>
)

if(REALM_ENABLE_LOGGING)
    target_compile_definitions(realm_compiler_options INTERFACE REALM_ENABLE_LOGGING)
endif()
if(REALM_ENABLE_PROFILER)
    target_compile_definitions(realm_compiler_options INTERFACE REALM_ENABLE_PROFILER)
endif()

# Sanitizer options target
add_library(realm_sanitizer_options INTERFACE)
add_library(Realm::SanitizerOptions ALIAS realm_sanitizer_options)

if(REALM_ENABLE_SANITIZERS)
    if(MSVC)
        target_compile_options(realm_sanitizer_options INTERFACE /fsanitize=address)
    else()
        target_compile_options(realm_sanitizer_options INTERFACE
            -fsanitize=address,undefined -fno-omit-frame-pointer)
        target_link_options(realm_sanitizer_options INTERFACE
            -fsanitize=address,undefined)
    endif()
endif()
