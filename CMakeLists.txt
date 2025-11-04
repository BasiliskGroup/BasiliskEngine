cmake_minimum_required(VERSION 3.20)
project(BasiliskEngine LANGUAGES C CXX)

# ======================================================
# Build options
# ======================================================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(BASILISK_BUILD_DOCS "Build API documentation" OFF)

# ======================================================
# External dependencies via FetchContent
# ======================================================
include(FetchContent)

# ---- GLAD (OpenGL loader) ----
set(GLAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/glad)
if(EXISTS "${GLAD_DIR}/glad.c")
    message(STATUS "Using local GLAD source")
    add_library(glad STATIC
        ${GLAD_DIR}/glad.c
    )
    target_include_directories(glad
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
    )
    target_compile_definitions(glad PRIVATE GLAD_GLAPI_EXPORT)
else()
    message(STATUS "Fetching GLAD from remote...")
    FetchContent_Declare(
        glad
        GIT_REPOSITORY https://github.com/Dav1dde/glad.git
        GIT_TAG        master
    )
    FetchContent_MakeAvailable(glad)
endif()

# ---- GLM (header-only math) ----
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
)
FetchContent_MakeAvailable(glm)

add_library(glm_h INTERFACE)
target_include_directories(glm_h INTERFACE ${glm_SOURCE_DIR})

# ---- GLFW (window/input) ----
if(NOT TARGET glfw)
    message(STATUS "Fetching GLFW...")
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG        3.3.8
    )
    FetchContent_MakeAvailable(glfw)
endif()

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
find_package(OpenGL REQUIRED)

target_link_libraries(basilisk
    PUBLIC
        glfw
        glad
        OpenGL::GL
        assimp
        glm_h
        stb
)

# ======================================================
# Test executable (engine)
# ======================================================
set(BASILISK_MAIN ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)

if(EXISTS ${BASILISK_MAIN})
    add_executable(engine ${BASILISK_MAIN})

    target_link_libraries(engine
        PRIVATE
            basilisk
    )

    target_include_directories(engine
        PRIVATE
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    )

    set_target_properties(engine PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    )

    # Resource copying
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shaders DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/textures DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/models DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

    message(STATUS "Added test executable: engine")
else()
    message(WARNING "main.cpp not found — skipping engine build.")
endif()

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

# ======================================================
# Summary
# ======================================================
message(STATUS "")
message(STATUS "================= Basilisk Configuration =================")
message(STATUS "  C++ Standard:          ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Docs:            ${BASILISK_BUILD_DOCS}")
message(STATUS "==========================================================")
