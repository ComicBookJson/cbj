cmake_minimum_required(VERSION 3.14)
project(ComicBookJson VERSION 1.0.0 LANGUAGES C CXX)

# Exigir C++17 (necessário para std::optional usado no quicktype e RapidJSON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON) # Essencial para gerar DLL/SO compartilhado

# Habilitar o módulo de download de dependências do CMake
include(FetchContent)

# unarr v1.1.1 has a legacy CMakeLists.txt without a modern minimum version.
# Keep the dependency build compatible with current CMake releases.
set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "Minimum policy version for legacy dependencies")

# Apple packages are merged into the final architecture-specific static archive.
# unarr honors BUILD_SHARED_LIBS, so force its dependency to produce libunarr.a
# instead of a dylib on iOS and Mac Catalyst.
if(APPLE OR ANDROID)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build dependencies as static libraries" FORCE)
else()
    set(BUILD_SHARED_LIBS ON CACHE BOOL "Build dependencies as shared libraries" FORCE)
endif()

# unarr must be self-contained: use its bundled bzip2/lzma/zlib implementations
# instead of relying on platform package discovery.
set(USE_SYSTEM_BZ2 OFF CACHE BOOL "Use bundled bzip2 in unarr" FORCE)
set(USE_SYSTEM_LZMA OFF CACHE BOOL "Use bundled LZMA in unarr" FORCE)
set(USE_SYSTEM_ZLIB OFF CACHE BOOL "Use bundled zlib in unarr" FORCE)

# 1. Baixar o UNARR (Para CBR/RAR) na hora da compilação
FetchContent_Declare(
    unarr
    GIT_REPOSITORY https://github.com/selmf/unarr.git
    GIT_TAG        v1.1.1
)
FetchContent_MakeAvailable(unarr)

# 2. Chamar o arquivo CMake da pasta core
add_subdirectory(core)

# SWIG is the only dynamic SDK boundary. The wrapper consumes the native
# CbJCore/static third-party graph and exposes it to the selected language.
add_subdirectory(swig)