cmake_minimum_required(VERSION 3.14)
project(ComicBookJson SDK VERSION 1.0.0 LANGUAGES C CXX)

# Exigir C++17 (necessário para std::optional usado no quicktype e nlohmann)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON) # Essencial para gerar DLL/SO compartilhado

# Habilitar o módulo de download de dependências do CMake
include(FetchContent)

# 1. Baixar o UNARR (Para CBR/RAR) na hora da compilação
FetchContent_Declare(
    unarr
    GIT_REPOSITORY https://github.com/selmf/unarr.git
    GIT_TAG        v1.1.1
)
FetchContent_MakeAvailable(unarr)

# 2. Chamar o arquivo CMake da pasta core
add_subdirectory(core)

# (Futuro: add_subdirectory(swig) para gerar os bindings C#, Java, Swift)