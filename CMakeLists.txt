cmake_minimum_required(VERSION 3.14)
project(Basilisk)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ======================================================
# Dependencies
# ======================================================

# glad
add_library(glad STATIC include/glad/glad.c)
target_include_directories(glad PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

# stb (header-only)
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

# External deps (glfw, glm, assimp)
add_subdirectory(include)

# ======================================================
# Library target (Basilisk)
# ======================================================

# Collect all .cpp source files
file(GLOB_RECURSE ENGINE_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

# Remove test main.cpp if it exists
list(REMOVE_ITEM ENGINE_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

add_library(basilisk STATIC ${ENGINE_SRC})

# Only public include directory now — all headers live under include/basilisk/
target_include_directories(basilisk
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(basilisk
    PUBLIC
        glad
        glfw
        glm
        assimp
        stb
)

# ======================================================
# Example / test executable
# ======================================================

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    add_executable(engine ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)
    target_link_libraries(engine PRIVATE basilisk)

    # Copy resources for runtime
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shaders DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/textures DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/models DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()

# ======================================================
# Install rules (optional, good for packaging / FetchContent)
# ======================================================

install(TARGETS basilisk
    EXPORT BasiliskTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
install(DIRECTORY include/ DESTINATION include)

install(EXPORT BasiliskTargets
    FILE BasiliskTargets.cmake
    NAMESPACE Basilisk::
    DESTINATION lib/cmake/Basilisk
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/BasiliskConfigVersion.cmake"
    VERSION 1.0.0
    COMPATIBILITY SameMajorVersion
)

install(FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/BasiliskConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/BasiliskConfigVersion.cmake"
    DESTINATION lib/cmake/Basilisk
)
