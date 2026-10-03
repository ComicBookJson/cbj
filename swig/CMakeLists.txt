cmake_minimum_required(VERSION 3.14)

# SWIG is the sole dynamic language boundary. The native SDK and all third-party
# libraries remain linked below it, so consumers load only the generated module.
find_package(SWIG 4 REQUIRED)
include(UseSWIG)

set(CMAKE_SWIG_OUTDIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(UseSWIG_TARGET_NAME_PREFERENCE STANDARD)

function(cbj_add_swig_target target language)
    set_property(SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/cbj.i" PROPERTY CPLUSPLUS ON)

    swig_add_library(${target}
        TYPE SHARED
        LANGUAGE ${language}
        OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/${target}"
        OUTFILE_DIR "${CMAKE_CURRENT_BINARY_DIR}/${target}"
        SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/cbj.i"
    )

    set_property(TARGET ${target} PROPERTY SWIG_USE_TARGET_INCLUDE_DIRECTORIES TRUE)
    target_include_directories(${target} PRIVATE
        "${PROJECT_SOURCE_DIR}/core/include"
        "${PROJECT_SOURCE_DIR}/core/vendor"
    )
    target_link_libraries(${target} PRIVATE CbjCore)

    if(WIN32)
        # The wrapper is the only DLL; CbJCore and all third-party code are
        # statically linked into this module.
        set_target_properties(${target} PROPERTIES PREFIX "")
    endif()

    # Publish the architecture-specific artifacts in the repository dist tree.
    if(ANDROID)
        set(CBJ_DIST_PLATFORM "android")
        set(CBJ_DIST_ABI "${CMAKE_ANDROID_ARCH_ABI}")
    elseif(APPLE)
        if(DEFINED MACCATALYST OR CMAKE_C_COMPILER_TARGET MATCHES "-macabi$")
            set(CBJ_DIST_PLATFORM "maccatalyst")
        elseif(CMAKE_SYSTEM_NAME STREQUAL "iOS")
            set(CBJ_DIST_PLATFORM "ios")
        else()
            set(CBJ_DIST_PLATFORM "macos")
        endif()
        set(CBJ_DIST_ABI "${CMAKE_OSX_ARCHITECTURES}")
        string(REPLACE ";" "-" CBJ_DIST_ABI "${CBJ_DIST_ABI}")
    elseif(WIN32)
        set(CBJ_DIST_PLATFORM "windows")
        if(CMAKE_GENERATOR_PLATFORM)
            set(CBJ_DIST_ABI "${CMAKE_GENERATOR_PLATFORM}")
        else()
            set(CBJ_DIST_ABI "${CMAKE_SYSTEM_PROCESSOR}")
        endif()
    else()
        set(CBJ_DIST_PLATFORM "${CMAKE_SYSTEM_NAME}")
        set(CBJ_DIST_ABI "${CMAKE_SYSTEM_PROCESSOR}")
    endif()
    if(NOT CBJ_DIST_ABI)
        set(CBJ_DIST_ABI "default")
    endif()
    set(CBJ_DIST_DIR "${PROJECT_SOURCE_DIR}/dist/${CBJ_DIST_PLATFORM}/${CBJ_DIST_ABI}")

    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${CBJ_DIST_DIR}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:${target}>" "${CBJ_DIST_DIR}/$<TARGET_FILE_NAME:${target}>"
        VERBATIM)

    if(ANDROID)
        file(GLOB_RECURSE CBJ_LIBCXX_SHARED_FILES
            "${CMAKE_SYSROOT}/usr/lib/*/libc++_shared.so")
        list(LENGTH CBJ_LIBCXX_SHARED_FILES CBJ_LIBCXX_SHARED_COUNT)
        if(CBJ_LIBCXX_SHARED_COUNT GREATER 0)
            list(GET CBJ_LIBCXX_SHARED_FILES 0 CBJ_LIBCXX_SHARED)
            add_custom_command(TARGET ${target} POST_BUILD
                COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CBJ_LIBCXX_SHARED}" "${CBJ_DIST_DIR}/libc++_shared.so"
                VERBATIM)
        endif()
    endif()

    if(APPLE)
        # Merge all static archives produced by this architecture into libcbz.a.
        # The build tree is architecture-specific, so *.a never mixes ABIs.
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND /bin/sh -c "set -e; mkdir -p '${CBJ_DIST_DIR}'; rm -f '${CBJ_DIST_DIR}/libcbz.a'; find '${CMAKE_BINARY_DIR}' -type f -name '*.a' -exec cp -f {} '${CBJ_DIST_DIR}/' \\;; cd '${CBJ_DIST_DIR}'; libtool -static -o libcbz.a *.a"
            VERBATIM)
    endif()

    if("${language}" STREQUAL "java")
        # Java's generated proxy loads the native module as "cbz".
        set_target_properties(${target} PROPERTIES OUTPUT_NAME "cbz")
    elseif("${language}" STREQUAL "csharp")
        set_target_properties(${target} PROPERTIES OUTPUT_NAME "cbz_csharp")
    endif()
endfunction()

cbj_add_swig_target(cbj_swig_csharp csharp)

if(ANDROID)
    cbj_add_swig_target(cbj_swig_java java)
endif()
