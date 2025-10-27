cmake_minimum_required(VERSION 3.14)
project(Basilisk)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -----------------------
# Dependencies
# -----------------------

add_library(glad STATIC include/glad/glad.c)
target_include_directories(glad PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

include(FetchContent)
add_subdirectory(include)

# -----------------------
# Library target
# -----------------------

file(GLOB_RECURSE ENGINE_SRC 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/**/*.cpp
)

# Remove main.cpp if it exists
list(REMOVE_ITEM ENGINE_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

add_library(basilisk STATIC ${ENGINE_SRC})

target_include_directories(basilisk PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(basilisk
    glad
    glfw
    glm 
    assimp
    stb
    xsimd
)

target_compile_options(basilisk PRIVATE
    -O3
    -ffast-math
    -march=native
)

# -----------------------
# Test executable
# -----------------------

add_executable(engine
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
)

target_link_libraries(engine PRIVATE basilisk)

# -----------------------
# Resource copying
# -----------------------

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/shaders)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shaders DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/textures)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/textures DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/models)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/models DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# -----------------------
# Debug (AddressSanitizer)
# -----------------------

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(basilisk PUBLIC -fsanitize=address -fno-omit-frame-pointer -g)
    target_link_options(basilisk PUBLIC -fsanitize=address -fno-omit-frame-pointer)
endif()

# -----------------------
# Documentation
# -----------------------

find_package(Doxygen OPTIONAL_COMPONENTS dot)

if(DOXYGEN_FOUND)
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.out)

    add_custom_target(doc
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_IN}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )
else()
    message(STATUS "Doxygen not found, 'doc' target will not be available.")
endif()
