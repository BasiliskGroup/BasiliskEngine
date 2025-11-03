cmake_minimum_required(VERSION 3.14)
project(Basilisk)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ======================================================
# Copy basilisk.h to include/basilisk/ for public use
# ======================================================
set(BASILISK_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(BASILISK_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/basilisk)

file(MAKE_DIRECTORY ${BASILISK_INCLUDE_DIR})

# Copy the main basilisk.h header
configure_file(
    ${BASILISK_SRC_DIR}/basilisk.h
    ${BASILISK_INCLUDE_DIR}/basilisk.h
    COPYONLY
)

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

file(GLOB_RECURSE ENGINE_SRC 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

# Remove test main.cpp
list(REMOVE_ITEM ENGINE_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

add_library(basilisk STATIC ${ENGINE_SRC})

# Both include/ AND src/ must be PUBLIC so basilisk.h can find internal headers
target_include_directories(basilisk
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)

target_link_libraries(basilisk
    PUBLIC
        glad
        glfw
        glm
        assimp
        stb
)

# REMOVED: AddressSanitizer is no longer applied to the library
# Users can enable it in their own projects if needed

# ======================================================
# Test executable (only when building as main project)
# ======================================================

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    add_executable(engine ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)
    target_link_libraries(engine PRIVATE basilisk)
    
    # Optional: Enable ASan for YOUR test executable only
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU" AND NOT CMAKE_CROSSCOMPILING)
        target_compile_options(engine PRIVATE -fsanitize=address -fno-omit-frame-pointer -g)
        target_link_options(engine PRIVATE -fsanitize=address -fno-omit-frame-pointer)
    endif()

    # Resource copying
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shaders DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/textures DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/models DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()

# ======================================================
# Documentation (Doxygen)
# ======================================================

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    find_package(Doxygen OPTIONAL_COMPONENTS dot)
    if(DOXYGEN_FOUND)
        set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile)
        add_custom_target(doc
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_IN}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM 
        )
    endif()
endif()