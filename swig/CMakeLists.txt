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
        "${PROJECT_SOURCE_DIR}/core/vendor/rapidjson"
        "${PROJECT_SOURCE_DIR}/core/vendor/logfault"
        "${PROJECT_SOURCE_DIR}/core/vendor/miniz"
    )
    target_link_libraries(${target} PRIVATE CbjCore)

    if(WIN32)
        # The wrapper is the only DLL; CbJCore and all third-party code are
        # statically linked into this module.
        set_target_properties(${target} PROPERTIES PREFIX "")
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
