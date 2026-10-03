set(CORE_SOURCES
    src/Cbj.cpp
    src/CbjBuilder.cpp
    src/CbjLog.cpp
    src/CbjJsonStream.cpp
    src/StringImageHelper.cpp
    vendor/miniz/miniz.c
)

if(DEFINED CBJ_CSHARP_SWIG_WRAPPER AND EXISTS "${CBJ_CSHARP_SWIG_WRAPPER}")
    list(APPEND CORE_SOURCES "${CBJ_CSHARP_SWIG_WRAPPER}")
endif()
if(DEFINED CBJ_JAVA_SWIG_WRAPPER AND EXISTS "${CBJ_JAVA_SWIG_WRAPPER}")
    list(APPEND CORE_SOURCES "${CBJ_JAVA_SWIG_WRAPPER}")
endif()

if(WIN32 OR APPLE)
    add_library(CbjCore STATIC ${CORE_SOURCES})
else()
    add_library(CbjCore SHARED ${CORE_SOURCES})
endif()

set_target_properties(CbjCore PROPERTIES OUTPUT_NAME cbz)
if(WIN32)
    set_target_properties(CbjCore PROPERTIES PREFIX "")
endif()
if(MSVC)
    target_compile_options(CbjCore PRIVATE /EHsc)
endif()

target_include_directories(CbjCore PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_include_directories(CbjCore PRIVATE vendor/rapidjson vendor/logfault vendor/miniz)
target_link_libraries(CbjCore PRIVATE unarr)

# RapidJSON and Logfault are vendored under core/vendor.
include(FetchContent)

FetchContent_Declare(
    mupdf
    GIT_REPOSITORY https://github.com/ArtifexSoftware/mupdf.git
    GIT_TAG 1.28.5
    GIT_SHALLOW TRUE
    GIT_SUBMODULES_RECURSE TRUE
)
FetchContent_MakeAvailable(mupdf)

set(MUPDF_BUILD_DIR "${mupdf_BINARY_DIR}/build/release")
if(WIN32)
    # MuPDF's official Visual Studio solution produces static .lib files.
    if(CMAKE_GENERATOR_PLATFORM STREQUAL "ARM64")
        set(MUPDF_VS_PLATFORM "ARM64")
    elseif(CMAKE_GENERATOR_PLATFORM STREQUAL "Win32")
        set(MUPDF_VS_PLATFORM "Win32")
    else()
        set(MUPDF_VS_PLATFORM "x64")
    endif()
    set(MUPDF_WIN_LIB_DIR "${mupdf_SOURCE_DIR}/platform/win32/${MUPDF_VS_PLATFORM}/Release")
    add_custom_target(cbj_mupdf_build
        COMMAND msbuild "${mupdf_SOURCE_DIR}/platform/win32/mupdf.sln"
                /m: /p:Configuration=Release /p:Platform=${MUPDF_VS_PLATFORM}
                /t:libmupdf
        BYPRODUCTS
            "${MUPDF_WIN_LIB_DIR}/libmupdf.lib"
            "${MUPDF_WIN_LIB_DIR}/libthirdparty.lib"
        VERBATIM)
    add_dependencies(CbjCore cbj_mupdf_build)
    target_include_directories(CbjCore PRIVATE "${mupdf_SOURCE_DIR}/include")
    target_link_libraries(CbjCore PRIVATE
        "${MUPDF_WIN_LIB_DIR}/libmupdf.lib"
        "${MUPDF_WIN_LIB_DIR}/libthirdparty.lib"
    )
elseif(UNIX OR APPLE)
    add_custom_target(cbj_mupdf_build
        COMMAND make -C "${mupdf_SOURCE_DIR}" build=release HAVE_X11=no HAVE_GLFW=no
        BYPRODUCTS "${MUPDF_BUILD_DIR}/libmupdf.a" "${MUPDF_BUILD_DIR}/libmupdf-third.a"
        VERBATIM)
    add_dependencies(CbjCore cbj_mupdf_build)
    target_include_directories(CbjCore PRIVATE "${mupdf_SOURCE_DIR}/include")
    target_link_libraries(CbjCore PRIVATE
        "${MUPDF_BUILD_DIR}/libmupdf.a"
        "${MUPDF_BUILD_DIR}/libmupdf-third.a"
    )
else()
    message(FATAL_ERROR "Unsupported platform for MuPDF source build.")
endif()

# MuPDF may include GLX headers on Unix desktop; expose the system GLX include
# directory only where GLX is actually part of the platform.
if(UNIX AND NOT APPLE AND NOT ANDROID)
    find_path(CBJ_GLX_INCLUDE_DIR GL/glx.h)
    if(CBJ_GLX_INCLUDE_DIR)
        target_include_directories(CbjCore PRIVATE "${CBJ_GLX_INCLUDE_DIR}")
    endif()
endif()

if(ANDROID)
    target_link_options(CbjCore PRIVATE
        "-Wl,-z,max-page-size=16384"
        "-Wl,-z,common-page-size=16384"
    )
endif()
