set(CORE_SOURCES
    src/Cbj.cpp
    vendor/miniz/miniz.c
)

add_library(CbjzCore SHARED ${CORE_SOURCES})

# Dizer ao compilador onde encontrar os headers (arquivos .hpp / .h)
target_include_directories(CbjzCore PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Adicionar a pasta vendor aos includes (para acharmos json.hpp e miniz.h)
target_include_directories(CbjzCore PRIVATE
    vendor/json
    vendor/miniz
)

# Linkar as bibliotecas baixadas via FetchContent ao nosso Core
target_link_libraries(CbjzCore PRIVATE unarr)

# =========================================================
# Lógica para baixar o PDFium pré-compilado dinamicamente
# dependendo do Sistema Operacional (Windows, Mac ou Linux)
# =========================================================
if(WIN32)
    set(PDFIUM_URL "https://github.com/bblanchon/pdfium-binaries/releases/latest/download/pdfium-windows-x64.tgz")
elseif(APPLE)
    set(PDFIUM_URL "https://github.com/bblanchon/pdfium-binaries/releases/latest/download/pdfium-mac-arm64.tgz") # Ajuste arm64/x64 se necessário
else()
    set(PDFIUM_URL "https://github.com/bblanchon/pdfium-binaries/releases/latest/download/pdfium-linux-x64.tgz")
endif()

FetchContent_Declare(
    pdfium
    URL ${PDFIUM_URL}
)
FetchContent_MakeAvailable(pdfium)

target_include_directories(CbjzCore PRIVATE ${pdfium_SOURCE_DIR}/include)
target_link_directories(CbjzCore PRIVATE ${pdfium_SOURCE_DIR}/lib)
target_link_libraries(CbjzCore PRIVATE pdfium)