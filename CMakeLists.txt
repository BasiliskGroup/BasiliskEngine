cmake_minimum_required(VERSION 3.20)
project(BasiliskEngine LANGUAGES C CXX)

# ======================================================
# Build settings
# ======================================================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)


# Architecture settings (can be overridden with -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64" for universal)
# Options: "arm64", "x86_64", or "x86_64;arm64" for universal
if(NOT DEFINED CMAKE_OSX_ARCHITECTURES)
    # Default to system architecture if not specified
    # To build universal: cmake -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64" ..
    # To build x86_64: cmake -DCMAKE_OSX_ARCHITECTURES="x86_64" ..
    # To build arm64: cmake -DCMAKE_OSX_ARCHITECTURES="arm64" ..
endif()

option(BASILISK_BUILD_DOCS "Build API documentation" OFF)

include(FetchContent)

# ======================================================
# GLAD
# ======================================================
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
        GIT_TAG master
    )
    FetchContent_MakeAvailable(glad)
endif()

# ======================================================
# GLM (header-only)
# ======================================================
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.3
)
FetchContent_MakeAvailable(glm)

add_library(glm_h INTERFACE)
target_include_directories(glm_h INTERFACE ${glm_SOURCE_DIR})

# ======================================================
# GLFW
# ======================================================
if(NOT TARGET glfw)
    message(STATUS "Fetching GLFW...")
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.3.8
    )
    FetchContent_MakeAvailable(glfw)
endif()

# ======================================================
# STB (header-only)
# ======================================================
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/stb
)

# ======================================================
# Assimp
# ======================================================
# Use system zlib instead of bundled zlib to avoid macOS compatibility issues
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)

# Find system zlib
if(WIN32)
    # Windows CI does not provide system zlib
    set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "Build zlib" FORCE)
else()
    # Linux/macOS use system zlib
    set(ASSIMP_BUILD_ZLIB OFF CACHE BOOL "Build zlib" FORCE)
    find_package(ZLIB REQUIRED)
endif()

FetchContent_Declare(
    assimp
    GIT_REPOSITORY https://github.com/assimp/assimp.git
    GIT_TAG v6.0.2
)
FetchContent_MakeAvailable(assimp)

# ======================================================
# Clipper2 (static library the same way CrumpleQuest does)
# ======================================================
FetchContent_Declare(
    clipper2
    GIT_REPOSITORY https://github.com/AngusJohnson/Clipper2.git
    GIT_TAG main
)
FetchContent_MakeAvailable(clipper2)

# ======================================================
# PyBind11
# ======================================================
# Find Python before pybind11 to ensure correct Python is used
# Users can override with: -DPYTHON_EXECUTABLE=/path/to/python3

# Find Python3 package (will use PYTHON_EXECUTABLE if set)
find_package(Python3 COMPONENTS Interpreter Development QUIET)
if(Python3_FOUND)
    message(STATUS "Found Python ${Python3_VERSION}: ${Python3_EXECUTABLE}")
    message(STATUS "  Python include dirs: ${Python3_INCLUDE_DIRS}")
    # Ensure pybind11 uses the same Python
    set(PYTHON_EXECUTABLE ${Python3_EXECUTABLE} CACHE PATH "Python executable" FORCE)
else()
    message(WARNING "Python3 not found - pybind11 will attempt to find it automatically")
endif()

FetchContent_Declare(
    pybind11
    GIT_REPOSITORY https://github.com/pybind/pybind11.git
    GIT_TAG v3.0.1
)
FetchContent_MakeAvailable(pybind11)

file(GLOB CLIPPER2_SOURCES
    ${clipper2_SOURCE_DIR}/CPP/Clipper2Lib/src/*.cpp
)

add_library(clipper2 STATIC ${CLIPPER2_SOURCES})

target_include_directories(clipper2 PUBLIC
    ${clipper2_SOURCE_DIR}/CPP/Clipper2Lib/include
)

target_compile_features(clipper2 PUBLIC cxx_std_20)

# ======================================================
# Basilisk Engine Sources
# ======================================================
file(GLOB_RECURSE BASILISK_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

# Remove main.cpp since engine executable builds it
list(REMOVE_ITEM BASILISK_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

add_library(basilisk_lib STATIC ${BASILISK_SOURCES})

target_include_directories(basilisk_lib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/internal
)

# ======================================================
# Link Dependencies to Basilisk
# ======================================================
find_package(OpenGL REQUIRED)

target_link_libraries(basilisk_lib
    PUBLIC
        glfw
        glad
        OpenGL::GL
        assimp
        glm_h
        stb
        clipper2
)

if(MSVC)
    target_compile_options(basilisk_lib PRIVATE /FS)
endif()

# ======================================================
# Python Bindings
# ======================================================
file(GLOB_RECURSE BINDING_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/bindings/*.cpp
)

pybind11_add_module(basilisk ${BINDING_SOURCES})

target_include_directories(basilisk PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/bindings
)

target_link_libraries(basilisk PRIVATE
    basilisk_lib
)

# ======================================================
# Generate Python type stubs with pybind11-stubgen
# ======================================================
# Find Python3 to get the interpreter
find_package(Python3 COMPONENTS Interpreter QUIET)

if(Python3_FOUND AND TARGET basilisk)
    # Determine the output directory for stubs (in the source tree, will be packaged)
    set(STUB_OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/basilisk)
    
    # Check if pybind11-stubgen is available
    execute_process(
        COMMAND ${Python3_EXECUTABLE} -c "import pybind11_stubgen; print('OK')"
        OUTPUT_QUIET
        ERROR_QUIET
        RESULT_VARIABLE STUBGEN_AVAILABLE
    )
    
    if(STUBGEN_AVAILABLE EQUAL 0)
        # Create output directory (in source tree so it gets packaged)
        file(MAKE_DIRECTORY ${STUB_OUTPUT_DIR})
        
        # Custom command to generate stubs after module is built
        # We need to add the build directory to PYTHONPATH so stubgen can import the module
        # The stub will be generated as basilisk.pyi in the output directory
        # Note: This may fail if runtime dependencies (like glm) are not available
        # during build, so we use a wrapper script to make errors non-fatal
        add_custom_command(TARGET basilisk POST_BUILD
            COMMAND ${Python3_EXECUTABLE}
                ${CMAKE_CURRENT_SOURCE_DIR}/cmake/run_stubgen.py
                ${CMAKE_CURRENT_BINARY_DIR}
                ${STUB_OUTPUT_DIR}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating Python type stubs for basilisk module (non-fatal)"
        )
        
        message(STATUS "Python type stub generation enabled")
        message(STATUS "  Stubs will be generated in: ${STUB_OUTPUT_DIR}/basilisk.pyi")
        message(STATUS "  Note: Stub generation is non-fatal and may fail if runtime dependencies are missing")
    else()
        message(WARNING "pybind11-stubgen not available - type stubs will not be generated")
        message(WARNING "  Install with: pip install pybind11-stubgen")
    endif()
endif()

# ======================================================
# Engine executable
# ======================================================
set(BASILISK_MAIN ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)

if(EXISTS ${BASILISK_MAIN})
    add_executable(engine ${BASILISK_MAIN})

    target_include_directories(engine PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

    target_link_libraries(engine
        PRIVATE
            basilisk_lib
            clipper2
    )

    set_target_properties(engine PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    )

    # Copy resources
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shaders  DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/textures DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/models   DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

    message(STATUS "Added test executable: engine")
else()
    message(WARNING "main.cpp not found — skipping engine build.")
endif()

# ======================================================
# AddressSanitizer
# ======================================================
# Only enable AddressSanitizer for Debug builds (not for Release/wheel builds)
# Note: AddressSanitizer requires the sanitizer runtime library, which may not be
# available in all build environments (e.g., cibuildwheel). For wheel builds,
# AddressSanitizer is disabled.
if(TARGET engine AND CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU" AND NOT CMAKE_CROSSCOMPILING)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(engine PRIVATE
            -fsanitize=address
            -fno-omit-frame-pointer
            -g
        )
        target_link_options(engine PRIVATE
            -fsanitize=address
            -fno-omit-frame-pointer
        )
        set(ENV{ASAN_OPTIONS} "detect_leaks=1:halt_on_error=0:verbosity=1")
    endif()
endif()

# ======================================================
# Optional Docs
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

# ======================================================
# Install
# ======================================================
install(TARGETS basilisk
    LIBRARY DESTINATION basilisk
    RUNTIME DESTINATION basilisk
)

# Add internal files 
install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/shaders
    DESTINATION basilisk
)