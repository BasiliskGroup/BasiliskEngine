cmake_minimum_required(VERSION 3.20)
project(BasiliskEngine LANGUAGES CXX)

# ======================================================
# Build options
# ======================================================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(BASILISK_BUILD_DOCS "Build API documentation" OFF)
option(BASILISK_BUILD_TESTS "Build Basilisk tests" OFF)
option(BASILISK_BUILD_EXAMPLES "Build Basilisk examples" OFF)

# ======================================================
# External dependencies via FetchContent
# ======================================================
include(FetchContent)

# ---- GLAD (OpenGL loader) ----
FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG        master
)
FetchContent_MakeAvailable(glad)

# ---- GLM (header-only math) ----
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
)
FetchContent_MakeAvailable(glm)

# Create INTERFACE target for GLM
add_library(glm_h INTERFACE)
target_include_directories(glm_h INTERFACE ${glm_SOURCE_DIR})

# ---- GLFW (window/input) ----
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.3.8
)
FetchContent_MakeAvailable(glfw)

# ---- STB (local header-only) ----
add_library(stb INTERFACE)
target_include_directories(stb
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/stb>
)

# ---- Assimp (3D model loading) ----
FetchContent_Declare(
    assimp
    GIT_REPOSITORY https://github.com/assimp/assimp.git
    GIT_TAG        v6.0.2
)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(assimp)

# ======================================================
# Basilisk sources
# ======================================================
file(GLOB_RECURSE BASILISK_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)
list(REMOVE_ITEM BASILISK_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
add_library(basilisk STATIC ${BASILISK_SOURCES})

# ======================================================
# Include directories for Basilisk
# ======================================================
target_include_directories(basilisk
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/internal>
)

# ======================================================
# Link dependencies
# ======================================================
target_link_libraries(basilisk
    PUBLIC
        glfw
        assimp
        glm_h
        stb
    PRIVATE
        glad
)

# ======================================================
# Compiler warnings
# ======================================================
# if(MSVC)
#     target_compile_options(basilisk PRIVATE /W4 /permissive-)
# else()
#     target_compile_options(basilisk PRIVATE -Wall -Wextra -Wpedantic)
# endif()

# ======================================================
# Optional docs/tests/examples
# ======================================================
if(BASILISK_BUILD_DOCS)
    find_package(Doxygen 1.9.8)
    if(DOXYGEN_FOUND)
        add_subdirectory(docs)
    else()
        message(WARNING "Doxygen not found, skipping docs")
    endif()
endif()

if(BASILISK_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(BASILISK_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# ======================================================
# Summary
# ======================================================
message(STATUS "")
message(STATUS "================= Basilisk Configuration =================")
message(STATUS "  C++ Standard:          ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Docs:            ${BASILISK_BUILD_DOCS}")
message(STATUS "  Build Tests:           ${BASILISK_BUILD_TESTS}")
message(STATUS "  Build Examples:        ${BASILISK_BUILD_EXAMPLES}")
message(STATUS "==========================================================")
