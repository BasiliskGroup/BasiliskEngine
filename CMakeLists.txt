cmake_minimum_required(VERSION 3.20)
project(BasiliskEngine LANGUAGES C CXX)

# ======================================================
# Build settings
# ======================================================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)


set(CMAKE_BUILD_TYPE Debug)


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

# Mapbox Earcut (header-only) bundled via Assimp's contrib
set(ASSIMP_EARCUT_INCLUDE_DIR "${CMAKE_BINARY_DIR}/_deps/assimp-src/contrib/earcut-hpp")

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

# ======================================================
# Rust GPU library
# ======================================================
# Rust produces different library names on different platforms:
# - macOS: librust_gpu.a (static)
# - Linux: librust_gpu.a (static)
# - Windows: rust_gpu.lib (static)
if(APPLE)
    set(RUST_GPU_LIB_NAME "librust_gpu.a")
elseif(UNIX)
    set(RUST_GPU_LIB_NAME "librust_gpu.a")
elseif(WIN32)
    set(RUST_GPU_LIB_NAME "rust_gpu.lib")
else()
    set(RUST_GPU_LIB_NAME "")
endif()

# Check if cargo is available and rust_gpu directory exists
find_program(CARGO cargo)
if(RUST_GPU_LIB_NAME AND CARGO AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/rust_gpu/Cargo.toml)
    set(RUST_GPU_BYPRODUCTS
        ${CMAKE_CURRENT_SOURCE_DIR}/rust_gpu/target/release/${RUST_GPU_LIB_NAME}
    )
    
    add_custom_target(rust_gpu_lib ALL
        COMMAND cargo build --release
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/rust_gpu
        BYPRODUCTS ${RUST_GPU_BYPRODUCTS}
        COMMENT "Building Rust GPU compute library (static)..."
    )
    
    # Create an IMPORTED static library target
    add_library(rust_gpu_imported STATIC IMPORTED GLOBAL)
    set_target_properties(rust_gpu_imported PROPERTIES
        IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/rust_gpu/target/release/${RUST_GPU_LIB_NAME}"
    )
    add_dependencies(rust_gpu_imported rust_gpu_lib)
    
    # On Windows, we need to link additional system libraries that wgpu depends on
    if(WIN32)
        target_link_libraries(rust_gpu_imported INTERFACE 
            ws2_32 userenv advapi32 ntdll bcrypt d3dcompiler
        )
    endif()
    
    # On macOS, wgpu's Metal backend needs these frameworks (MTLCopyAllDevices, kCAGravityTopLeft, etc.)
    if(APPLE)
        target_link_libraries(rust_gpu_imported INTERFACE
            "-framework Metal"
            "-framework MetalKit"
            "-framework QuartzCore"
            "-framework CoreGraphics"
            "-framework IOKit"
        )
    endif()
    
    message(STATUS "Rust GPU library will be built: ${RUST_GPU_LIB_NAME}")
else()
    # Create a dummy target if Rust is not available
    add_custom_target(rust_gpu_lib
        COMMENT "Rust GPU library not available (cargo not found or rust_gpu/Cargo.toml missing)"
    )
    if(NOT CARGO)
        message(WARNING "cargo not found - Rust GPU library will not be built")
    endif()
    if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/rust_gpu/Cargo.toml)
        message(WARNING "rust_gpu/Cargo.toml not found - Rust GPU library will not be built")
    endif()
    set(RUST_GPU_LIB_NAME "")
endif()

# Set RUST_GPU_LIB for linking
if(RUST_GPU_LIB_NAME)
    set(RUST_GPU_LIB rust_gpu_imported)
else()
    set(RUST_GPU_LIB "")
endif()

# ======================================================
# Basilisk Engine Sources
# ======================================================
file(GLOB_RECURSE BASILISK_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

# Top-level src/*.cpp are executable entry points; exclude them from library sources
file(GLOB BASILISK_APP_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)
list(REMOVE_ITEM BASILISK_SOURCES ${BASILISK_APP_SOURCES})

add_library(basilisk_lib STATIC ${BASILISK_SOURCES})

target_include_directories(basilisk_lib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        ${ASSIMP_EARCUT_INCLUDE_DIR}
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
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(basilisk PRIVATE
    basilisk_lib
)

# Link Rust GPU library for compute bindings (gpu_init, gpu_buffer_*, gpu_shader_*)
# RUST_GPU_LIB is set earlier, right after RUST_GPU_LIB_NAME
# On Windows, we use manual DLL loading (gpuLoader.cpp) so we don't link against the DLL
# On other platforms, we link directly
# Link Rust GPU static library for compute bindings
# The static library is now embedded into the .pyd, no separate DLL needed
if(RUST_GPU_LIB_NAME)
    target_link_libraries(basilisk PRIVATE rust_gpu_imported)
    add_dependencies(basilisk rust_gpu_lib)
endif()

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
# Copy resources (shaders, textures, models) to build dir
# Single target avoids race when engine and engine_forces_test build in parallel (-j)
# ======================================================
add_custom_target(copy_resources ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders ${CMAKE_BINARY_DIR}/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/textures ${CMAKE_BINARY_DIR}/textures
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/models ${CMAKE_BINARY_DIR}/models
    COMMENT "Copying resources to build directory"
)

# ======================================================
# Executables (one target per top-level src/*.cpp)
# ======================================================
set(BASILISK_APP_TARGETS "")
if(BASILISK_APP_SOURCES)
    foreach(BASILISK_APP_SOURCE ${BASILISK_APP_SOURCES})
        get_filename_component(BASILISK_APP_TARGET ${BASILISK_APP_SOURCE} NAME_WE)
        add_executable(${BASILISK_APP_TARGET} ${BASILISK_APP_SOURCE})

        target_include_directories(${BASILISK_APP_TARGET} PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
        )

        target_link_libraries(${BASILISK_APP_TARGET}
            PRIVATE
                basilisk_lib
        )

        # Link Rust GPU static library for compute bindings
        if(RUST_GPU_LIB_NAME)
            target_link_libraries(${BASILISK_APP_TARGET} PRIVATE rust_gpu_imported)
            add_dependencies(${BASILISK_APP_TARGET} rust_gpu_lib)
        endif()

        set_target_properties(${BASILISK_APP_TARGET} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
        )

        add_dependencies(${BASILISK_APP_TARGET} copy_resources)
        list(APPEND BASILISK_APP_TARGETS ${BASILISK_APP_TARGET})
        message(STATUS "Added executable: ${BASILISK_APP_TARGET}")
    endforeach()
else()
    message(WARNING "No top-level src/*.cpp files found — skipping executable builds.")
endif()

# ======================================================
# Rust Dependency
# ======================================================
# Only add dependencies if Rust GPU library is available
if(RUST_GPU_LIB_NAME)
    add_dependencies(basilisk_lib rust_gpu_lib)
endif()

# ======================================================
# AddressSanitizer
# ======================================================
# Enable/disable ASan explicitly. Vulkan stacks on Linux can trip ASan inside libvulkan/driver code.
option(BASILISK_ENABLE_ASAN "Enable AddressSanitizer for Debug builds" ON)

# Only enable AddressSanitizer for Debug builds (not for Release/wheel builds)
# Note: AddressSanitizer requires the sanitizer runtime library, which may not be
# available in all build environments (e.g., cibuildwheel). For wheel builds,
# AddressSanitizer is disabled.
# Disable ASan in cibuildwheel environments (wheels don't need ASan)
if(DEFINED ENV{CIBUILDWHEEL})
    set(BASILISK_ENABLE_ASAN OFF)
    message(STATUS "AddressSanitizer disabled in cibuildwheel environment")
endif()

if(BASILISK_ENABLE_ASAN AND BASILISK_APP_TARGETS AND CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU" AND NOT CMAKE_CROSSCOMPILING)
    # Only enable ASan for Debug builds
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        foreach(BASILISK_APP_TARGET ${BASILISK_APP_TARGETS})
            target_compile_options(${BASILISK_APP_TARGET} PRIVATE
                -fsanitize=address
                -fno-omit-frame-pointer
                -g
            )
            target_link_options(${BASILISK_APP_TARGET} PRIVATE
                -fsanitize=address
                -fno-omit-frame-pointer
            )
        endforeach()
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

# Rust GPU is now statically linked into the basilisk module, no separate library to install

# Add internal files 
install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/shaders
    DESTINATION basilisk
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/models
    DESTINATION basilisk
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/textures
    DESTINATION basilisk
)

# CLI build script (basilisk file.py / basilisk-engine file.py)
install(
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/basilisk/build.py
    DESTINATION basilisk
)