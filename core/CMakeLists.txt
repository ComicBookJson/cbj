set(CORE_SOURCES
    src/Cbj.cpp
    src/CbjBuilder.cpp
    src/StringImageHelper.cpp
    vendor/miniz/miniz.c
)

# CI generates the appropriate SWIG wrapper before CMake configure.
if(DEFINED CBJ_CSHARP_SWIG_WRAPPER AND EXISTS "${CBJ_CSHARP_SWIG_WRAPPER}")
    list(APPEND CORE_SOURCES "${CBJ_CSHARP_SWIG_WRAPPER}")
endif()
if(DEFINED CBJ_JAVA_SWIG_WRAPPER AND EXISTS "${CBJ_JAVA_SWIG_WRAPPER}")
    list(APPEND CORE_SOURCES "${CBJ_JAVA_SWIG_WRAPPER}")
endif()

if(APPLE)
    # Apple platforms must consume a static archive in App Store applications.
    add_library(CbjzCore STATIC ${CORE_SOURCES})
else()
    add_library(CbjzCore SHARED ${CORE_SOURCES})
endif()

set_target_properties(CbjzCore PROPERTIES OUTPUT_NAME cbz)
if(WIN32)
    set_target_properties(CbjzCore PROPERTIES PREFIX "")
endif()
if(MSVC)
    target_compile_options(CbjzCore PRIVATE /EHsc)
endif()

target_include_directories(CbjzCore PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_include_directories(CbjzCore PRIVATE vendor/json vendor/miniz)
target_link_libraries(CbjzCore PRIVATE unarr)

# Android NDK r27 needs explicit ELF alignment for 16 KB page-size devices.
# Keep both page-size linker values aligned as recommended by Android.
if(ANDROID)
    target_link_options(CbjzCore PRIVATE
        "-Wl,-z,max-page-size=16384"
        "-Wl,-z,common-page-size=16384"
    )
endif()

# PDFium is supplied as a prebuilt binary for the exact target platform.
# CI sets CBJ_PDFIUM_URL; keep a host fallback for local builds.
if(NOT DEFINED CBJ_PDFIUM_URL OR CBJ_PDFIUM_URL STREQUAL "")
    if(WIN32)
        set(CBJ_PDFIUM_URL "https://github.com/bblanchon/pdfium-binaries/releases/latest/download/pdfium-win-x64.tgz")
    elseif(ANDROID)
        set(CBJ_PDFIUM_URL "https://github.com/bblanchon/pdfium-binaries/releases/latest/download/pdfium-android-arm64.tgz")
    elseif(CMAKE_SYSTEM_NAME STREQUAL "iOS")
        if(DEFINED CBJ_IOS_CATALYST AND CBJ_IOS_CATALYST)
            set(CBJ_PDFIUM_URL "https://github.com/bblanchon/pdfium-binaries/releases/latest/download/pdfium-ios-catalyst-arm64.tgz")
        else()
            set(CBJ_PDFIUM_URL "https://github.com/bblanchon/pdfium-binaries/releases/latest/download/pdfium-ios-device-arm64.tgz")
        endif()
    elseif(APPLE)
        set(CBJ_PDFIUM_URL "https://github.com/bblanchon/pdfium-binaries/releases/latest/download/pdfium-mac-arm64.tgz")
    else()
        set(CBJ_PDFIUM_URL "https://github.com/bblanchon/pdfium-binaries/releases/latest/download/pdfium-linux-x64.tgz")
    endif()
endif()

FetchContent_Declare(pdfium URL ${CBJ_PDFIUM_URL})
FetchContent_MakeAvailable(pdfium)

target_include_directories(CbjzCore PRIVATE ${pdfium_SOURCE_DIR}/include)

if(WIN32)
    # bblanchon's Windows package uses pdfium.dll.lib as the import
    # library for the shipped pdfium.dll (not pdfium.lib).
    target_link_libraries(CbjzCore PRIVATE ${pdfium_SOURCE_DIR}/lib/pdfium.dll.lib)
else()
    target_link_directories(CbjzCore PRIVATE ${pdfium_SOURCE_DIR}/lib)
    target_link_libraries(CbjzCore PRIVATE pdfium)
endif()
