set(CORE_SOURCES
    src/Cbj.cpp
    src/CbjBuilder.cpp
    src/StringImageHelper.cpp
    vendor/miniz/miniz.c
)
if(APPLE)
    # Apple platforms must consume a static archive in App Store applications.
    add_library(CbjzCore STATIC ${CORE_SOURCES})
else()
    add_library(CbjzCore SHARED ${CORE_SOURCES})
endif()

target_include_directories(CbjzCore PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_include_directories(CbjzCore PRIVATE vendor/json vendor/miniz)
target_link_libraries(CbjzCore PRIVATE unarr)

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
target_link_directories(CbjzCore PRIVATE ${pdfium_SOURCE_DIR}/lib)
target_link_libraries(CbjzCore PRIVATE pdfium)
