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
# Rust GPU library
# ======================================================
# Rust produces different library names on different platforms:
# - macOS: librust_gpu.dylib
# - Linux: librust_gpu.so
# - Windows: rust_gpu.dll (no "lib" prefix)
if(APPLE)
    set(RUST_GPU_LIB_NAME "librust_gpu.dylib")
elseif(UNIX)
    set(RUST_GPU_LIB_NAME "librust_gpu.so")
elseif(WIN32)
    set(RUST_GPU_LIB_NAME "rust_gpu.dll")
else()
    set(RUST_GPU_LIB_NAME "")
endif()

# Check if cargo is available and rust_gpu directory exists
find_program(CARGO cargo)
if(RUST_GPU_LIB_NAME AND CARGO AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/rust_gpu/Cargo.toml)
    # On Windows, Rust produces both .dll and .dll.lib (import library for MSVC)
    # or .dll and .lib (for GNU toolchain)
    if(WIN32)
        set(RUST_GPU_BYPRODUCTS
            ${CMAKE_CURRENT_SOURCE_DIR}/rust_gpu/target/release/${RUST_GPU_LIB_NAME}
            ${CMAKE_CURRENT_SOURCE_DIR}/rust_gpu/target/release/rust_gpu.dll.lib
            ${CMAKE_CURRENT_SOURCE_DIR}/rust_gpu/target/release/rust_gpu.lib
        )
    else()
        set(RUST_GPU_BYPRODUCTS
            ${CMAKE_CURRENT_SOURCE_DIR}/rust_gpu/target/release/${RUST_GPU_LIB_NAME}
        )
    endif()
    
    add_custom_target(rust_gpu_lib ALL
        COMMAND cargo build --release
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/rust_gpu
        BYPRODUCTS ${RUST_GPU_BYPRODUCTS}
        COMMENT "Building Rust GPU compute library..."
    )
    
    # On Windows, create an IMPORTED library target for easier linking
    if(WIN32)
        add_library(rust_gpu_imported SHARED IMPORTED)
        set_target_properties(rust_gpu_imported PROPERTIES
            IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/rust_gpu/target/release/${RUST_GPU_LIB_NAME}"
            IMPORTED_IMPLIB "${CMAKE_CURRENT_SOURCE_DIR}/rust_gpu/target/release/rust_gpu.dll.lib"
        )
        add_dependencies(rust_gpu_imported rust_gpu_lib)
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

# Set RUST_GPU_LIB path immediately after RUST_GPU_LIB_NAME is set
# so it's available for all targets (engine, engine_forces_test, basilisk)
if(RUST_GPU_LIB_NAME)
    if(WIN32)
        # On Windows with MSVC, Rust produces rust_gpu.dll.lib as the import library
        # Try both possible names in case the toolchain differs
        set(RUST_GPU_LIB_IMPORT "${CMAKE_CURRENT_SOURCE_DIR}/rust_gpu/target/release/rust_gpu.dll.lib")
        set(RUST_GPU_LIB_ALT "${CMAKE_CURRENT_SOURCE_DIR}/rust_gpu/target/release/rust_gpu.lib")
        # Use the .dll.lib version (MSVC standard), but we'll verify it exists at link time
        set(RUST_GPU_LIB "${RUST_GPU_LIB_IMPORT}")
    else()
        set(RUST_GPU_LIB "${CMAKE_CURRENT_SOURCE_DIR}/rust_gpu/target/release/${RUST_GPU_LIB_NAME}")
    endif()
else()
    set(RUST_GPU_LIB "")
endif()

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
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(basilisk PRIVATE
    basilisk_lib
)

# Link Rust GPU library for compute bindings (gpu_init, gpu_buffer_*, gpu_shader_*)
# RUST_GPU_LIB is set earlier, right after RUST_GPU_LIB_NAME
# On Windows, we use manual DLL loading (gpuLoader.cpp) so we don't link against the DLL
# On other platforms, we link directly
if(RUST_GPU_LIB_NAME)
    if(WIN32)
        # On Windows, don't link - we'll manually load the DLL at runtime
        # This allows the DLL to be found relative to the .pyd location
        add_dependencies(basilisk rust_gpu_lib)
        # Copy DLL to output directory so it can be found next to the .pyd
        add_custom_command(TARGET basilisk POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/rust_gpu/target/release/rust_gpu.dll"
                $<TARGET_FILE_DIR:basilisk>
        )
    else()
        # On non-Windows, link directly
        target_link_libraries(basilisk PRIVATE ${RUST_GPU_LIB})
        add_dependencies(basilisk rust_gpu_lib)
    endif()
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

    # Link Rust GPU library for compute bindings (gpu_init, gpu_buffer_*, gpu_shader_*)
    if(RUST_GPU_LIB_NAME)
        if(WIN32 AND TARGET rust_gpu_imported)
            target_link_libraries(engine PRIVATE rust_gpu_imported)
        else()
            target_link_libraries(engine PRIVATE ${RUST_GPU_LIB})
        endif()
        add_dependencies(engine rust_gpu_lib)
        # Copy DLL to output directory on Windows
        if(WIN32)
            add_custom_command(TARGET engine POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${CMAKE_CURRENT_SOURCE_DIR}/rust_gpu/target/release/rust_gpu.dll"
                    $<TARGET_FILE_DIR:engine>
            )
        endif()
    endif()

    set_target_properties(engine PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    )

    # Copy resources
    add_custom_command(TARGET engine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:engine>/shaders
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/textures $<TARGET_FILE_DIR:engine>/textures
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/models $<TARGET_FILE_DIR:engine>/models
        # Copy WGSL compute shaders from physics directory
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/src/physics/shaders/velocity.wgsl
            $<TARGET_FILE_DIR:engine>/shaders/velocity.wgsl
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/src/physics/shaders/primal.wgsl
            $<TARGET_FILE_DIR:engine>/shaders/primal.wgsl
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/src/physics/shaders/dual.wgsl
            $<TARGET_FILE_DIR:engine>/shaders/dual.wgsl
    )

    message(STATUS "Added test executable: engine")
else()
    message(WARNING "main.cpp not found — skipping engine build.")
endif()

# Forces test executable (joints, springs, motors, manifolds)
set(BASILISK_FORCES_TEST_MAIN ${CMAKE_CURRENT_SOURCE_DIR}/src/main_forces_test.cpp)
if(EXISTS ${BASILISK_FORCES_TEST_MAIN})
    add_executable(engine_forces_test ${BASILISK_FORCES_TEST_MAIN})
    target_include_directories(engine_forces_test PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    target_link_libraries(engine_forces_test
        PRIVATE
            basilisk_lib
            clipper2
    )
    # Link Rust GPU library for compute bindings (gpu_init, gpu_buffer_*, gpu_shader_*)
    if(RUST_GPU_LIB_NAME)
        if(WIN32 AND TARGET rust_gpu_imported)
            target_link_libraries(engine_forces_test PRIVATE rust_gpu_imported)
        else()
            target_link_libraries(engine_forces_test PRIVATE ${RUST_GPU_LIB})
        endif()
        add_dependencies(engine_forces_test rust_gpu_lib)
        # Copy DLL to output directory on Windows
        if(WIN32)
            add_custom_command(TARGET engine_forces_test POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${CMAKE_CURRENT_SOURCE_DIR}/rust_gpu/target/release/rust_gpu.dll"
                    $<TARGET_FILE_DIR:engine_forces_test>
            )
        endif()
    endif()
    set_target_properties(engine_forces_test PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    )
    # Copy resources
    add_custom_command(TARGET engine_forces_test POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:engine_forces_test>/shaders
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/textures $<TARGET_FILE_DIR:engine_forces_test>/textures
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/models $<TARGET_FILE_DIR:engine_forces_test>/models
        # Copy WGSL compute shaders from physics directory
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/src/physics/shaders/velocity.wgsl
            $<TARGET_FILE_DIR:engine_forces_test>/shaders/velocity.wgsl
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/src/physics/shaders/primal.wgsl
            $<TARGET_FILE_DIR:engine_forces_test>/shaders/primal.wgsl
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/src/physics/shaders/dual.wgsl
            $<TARGET_FILE_DIR:engine_forces_test>/shaders/dual.wgsl
    )
    message(STATUS "Added forces test executable: engine_forces_test")
endif()

# ======================================================
# Rust Dependency
# ======================================================
# Only add dependencies if Rust GPU library is available
if(RUST_GPU_LIB_NAME)
    if(TARGET engine)
        add_dependencies(engine rust_gpu_lib)
    endif()
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

if(BASILISK_ENABLE_ASAN AND TARGET engine AND CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU" AND NOT CMAKE_CROSSCOMPILING)
    # Only enable ASan for Debug builds
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

# Install Rust GPU shared library alongside basilisk so it is findable at runtime (RPATH $ORIGIN)
if(RUST_GPU_LIB_NAME)
    if(WIN32)
        install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/rust_gpu/target/release/rust_gpu.dll
            DESTINATION basilisk
            OPTIONAL
        )
    elseif(UNIX AND NOT APPLE)
        install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/rust_gpu/target/release/librust_gpu.so
            DESTINATION basilisk
            OPTIONAL
        )
    endif()
endif()

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