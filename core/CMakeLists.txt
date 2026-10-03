set(CORE_SOURCES
    src/Cbj.cpp
    src/CbjBuilder.cpp
    src/StringImageHelper.cpp
    vendor/miniz/miniz.c
)
add_library(CbjzCore SHARED ${CORE_SOURCES})
target_include_directories(CbjzCore PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_include_directories(CbjzCore PRIVATE vendor/json vendor/miniz)
target_link_libraries(CbjzCore PRIVATE unarr)
if(WIN32)
 set(PDFIUM_URL "https://github.com/bblanchon/pdfium-binaries/releases/latest/download/pdfium-windows-x64.tgz")
elseif(APPLE)
 set(PDFIUM_URL "https://github.com/bblanchon/pdfium-binaries/releases/latest/download/pdfium-mac-arm64.tgz")
else()
 set(PDFIUM_URL "https://github.com/bblanchon/pdfium-binaries/releases/latest/download/pdfium-linux-x64.tgz")
endif()
FetchContent_Declare(pdfium URL ${PDFIUM_URL})
FetchContent_MakeAvailable(pdfium)
target_include_directories(CbjzCore PRIVATE ${pdfium_SOURCE_DIR}/include)
target_link_directories(CbjzCore PRIVATE ${pdfium_SOURCE_DIR}/lib)
target_link_libraries(CbjzCore PRIVATE pdfium)
