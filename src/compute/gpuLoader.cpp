#include <basilisk/compute/gpuWrapper.hpp>
#include <basilisk/util/resolvePath.h>
#include <iostream>
#include <stdexcept>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>

namespace bsk::internal {

// Function pointer types matching the Rust FFI
typedef void (*gpu_init_fn)();
typedef void* (*gpu_buffer_create_fn)(size_t);
typedef void (*gpu_buffer_destroy_fn)(void*);
typedef void (*gpu_buffer_write_fn)(void*, const uint32_t*, size_t);
typedef void (*gpu_buffer_read_fn)(void*, uint32_t*, size_t);
typedef void* (*gpu_shader_create_fn)(const char*, void**, size_t, uint64_t);
typedef void (*gpu_shader_destroy_fn)(void*);
typedef void (*gpu_shader_set_uniform_fn)(void*, const uint8_t*, size_t);
typedef void (*gpu_shader_dispatch_fn)(void*, uint32_t, uint32_t, uint32_t);

// Global DLL handle and function pointers
static HMODULE g_rust_gpu_dll = nullptr;
static bool g_dll_loaded = false;

// Function pointers
static gpu_init_fn g_gpu_init = nullptr;
static gpu_buffer_create_fn g_gpu_buffer_create = nullptr;
static gpu_buffer_destroy_fn g_gpu_buffer_destroy = nullptr;
static gpu_buffer_write_fn g_gpu_buffer_write = nullptr;
static gpu_buffer_read_fn g_gpu_buffer_read = nullptr;
static gpu_shader_create_fn g_gpu_shader_create = nullptr;
static gpu_shader_destroy_fn g_gpu_shader_destroy = nullptr;
static gpu_shader_set_uniform_fn g_gpu_shader_set_uniform = nullptr;
static gpu_shader_dispatch_fn g_gpu_shader_dispatch = nullptr;

// Load the Rust GPU DLL and get function pointers
static void loadRustGpuDll() {
    if (g_dll_loaded) {
        return;
    }

    try {
        // Get the directory where the .pyd is located
        std::filesystem::path module_dir = getModulePath();
        std::filesystem::path dll_path = module_dir / "rust_gpu.dll";

        // Try loading from module directory first
        g_rust_gpu_dll = LoadLibraryA(dll_path.string().c_str());

        // If that fails, try current directory
        if (!g_rust_gpu_dll) {
            dll_path = std::filesystem::current_path() / "rust_gpu.dll";
            g_rust_gpu_dll = LoadLibraryA(dll_path.string().c_str());
        }

        // If still fails, try system search paths (PATH)
        if (!g_rust_gpu_dll) {
            g_rust_gpu_dll = LoadLibraryA("rust_gpu.dll");
        }

        if (!g_rust_gpu_dll) {
            DWORD error = GetLastError();
            throw std::runtime_error(
                "Failed to load rust_gpu.dll. Tried:\n"
                "  - " + (module_dir / "rust_gpu.dll").string() + "\n"
                "  - " + (std::filesystem::current_path() / "rust_gpu.dll").string() + "\n"
                "  - System PATH\n"
                "Error code: " + std::to_string(error)
            );
        }

        // Get function pointers
        g_gpu_init = (gpu_init_fn)GetProcAddress(g_rust_gpu_dll, "gpu_init");
        g_gpu_buffer_create = (gpu_buffer_create_fn)GetProcAddress(g_rust_gpu_dll, "gpu_buffer_create");
        g_gpu_buffer_destroy = (gpu_buffer_destroy_fn)GetProcAddress(g_rust_gpu_dll, "gpu_buffer_destroy");
        g_gpu_buffer_write = (gpu_buffer_write_fn)GetProcAddress(g_rust_gpu_dll, "gpu_buffer_write");
        g_gpu_buffer_read = (gpu_buffer_read_fn)GetProcAddress(g_rust_gpu_dll, "gpu_buffer_read");
        g_gpu_shader_create = (gpu_shader_create_fn)GetProcAddress(g_rust_gpu_dll, "gpu_shader_create");
        g_gpu_shader_destroy = (gpu_shader_destroy_fn)GetProcAddress(g_rust_gpu_dll, "gpu_shader_destroy");
        g_gpu_shader_set_uniform = (gpu_shader_set_uniform_fn)GetProcAddress(g_rust_gpu_dll, "gpu_shader_set_uniform");
        g_gpu_shader_dispatch = (gpu_shader_dispatch_fn)GetProcAddress(g_rust_gpu_dll, "gpu_shader_dispatch");

        // Verify all functions were found
        if (!g_gpu_init || !g_gpu_buffer_create || !g_gpu_buffer_destroy ||
            !g_gpu_buffer_write || !g_gpu_buffer_read || !g_gpu_shader_create ||
            !g_gpu_shader_destroy || !g_gpu_shader_set_uniform || !g_gpu_shader_dispatch) {
            FreeLibrary(g_rust_gpu_dll);
            g_rust_gpu_dll = nullptr;
            throw std::runtime_error("Failed to get all required function pointers from rust_gpu.dll");
        }

        g_dll_loaded = true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading Rust GPU DLL: " << e.what() << std::endl;
        throw;
    }
}

// Wrapper functions that call through function pointers
void gpu_init_wrapper() {
    loadRustGpuDll();
    g_gpu_init();
}

void* gpu_buffer_create_wrapper(size_t len) {
    loadRustGpuDll();
    return g_gpu_buffer_create(len);
}

void gpu_buffer_destroy_wrapper(void* buffer) {
    loadRustGpuDll();
    g_gpu_buffer_destroy(buffer);
}

void gpu_buffer_write_wrapper(void* buffer, const uint32_t* ptr, size_t len) {
    loadRustGpuDll();
    g_gpu_buffer_write(buffer, ptr, len);
}

void gpu_buffer_read_wrapper(void* buffer, uint32_t* out_ptr, size_t len) {
    loadRustGpuDll();
    g_gpu_buffer_read(buffer, out_ptr, len);
}

void* gpu_shader_create_wrapper(const char* wgsl, void** buffers, size_t buffer_count, uint64_t uniform_size) {
    loadRustGpuDll();
    return g_gpu_shader_create(wgsl, buffers, buffer_count, uniform_size);
}

void gpu_shader_destroy_wrapper(void* shader) {
    loadRustGpuDll();
    g_gpu_shader_destroy(shader);
}

void gpu_shader_set_uniform_wrapper(void* shader, const uint8_t* ptr, size_t len) {
    loadRustGpuDll();
    g_gpu_shader_set_uniform(shader, ptr, len);
}

void gpu_shader_dispatch_wrapper(void* shader, uint32_t x, uint32_t y, uint32_t z) {
    loadRustGpuDll();
    g_gpu_shader_dispatch(shader, x, y, z);
}

} // namespace bsk::internal

#endif // _WIN32

