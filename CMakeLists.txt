cmake_minimum_required(VERSION 3.14)
project(Basilisk)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ======================================================
# Auto-copy all header files from src/ to include/basilisk
# ======================================================

set(BASILISK_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(BASILISK_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/basilisk)
file(MAKE_DIRECTORY ${BASILISK_INCLUDE_DIR})

# Copy util/includes.h to include/basilisk/
configure_file(
    ${BASILISK_SRC_DIR}/util/includes.h
    ${BASILISK_INCLUDE_DIR}/includes.h
    COPYONLY
)

# Copy util/print.h to include/basilisk/ (if you want it public too)
configure_file(
    ${BASILISK_SRC_DIR}/util/print.h
    ${BASILISK_INCLUDE_DIR}/print.h
    COPYONLY
)

# Copy constants.h if it exists in util/
if(EXISTS ${BASILISK_SRC_DIR}/util/constants.h)
    configure_file(
        ${BASILISK_SRC_DIR}/util/constants.h
        ${BASILISK_INCLUDE_DIR}/constants.h
        COPYONLY
    )
endif()

# ======================================================
# Dependencies
# ======================================================

# glad
add_library(glad STATIC include/glad/glad.c)
target_include_directories(glad PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# stb (header-only)
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# External deps (glm, glfw, assimp, xsimd)
include(FetchContent)
add_subdirectory(include)

# ======================================================
# Library target (Basilisk)
# ======================================================

file(GLOB_RECURSE ENGINE_SRC 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

# Remove test main.cpp (not part of the library)
list(REMOVE_ITEM ENGINE_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

add_library(basilisk STATIC ${ENGINE_SRC})

# Key change: expose both include/basilisk and src for headers
target_include_directories(basilisk
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>  # Add this
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

# target_compile_options(basilisk PRIVATE
#     -O3
#     -ffast-math
#     -march=native
# )

# AddressSanitizer (optional, only when building directly)
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU" AND NOT CMAKE_CROSSCOMPILING)
    target_compile_options(basilisk PUBLIC -fsanitize=address -fno-omit-frame-pointer -g)
    target_link_options(basilisk PUBLIC -fsanitize=address -fno-omit-frame-pointer)
endif()

# ======================================================
# Test executable (only when building as main project)
# ======================================================

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    add_executable(engine ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)
    target_link_libraries(engine PRIVATE basilisk)

    # Resource copying
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shaders DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/textures DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/models DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()

# ======================================================
# Documentation (Doxygen) - only when building as main project
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