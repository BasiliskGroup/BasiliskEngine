#ifndef BSK_GPU_WRAPPER
#define BSK_GPU_WRAPPER

#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>

// bound from Rust
extern "C" {
    void gpu_init();
    
    void* gpu_buffer_create(size_t len);
    void gpu_buffer_destroy(void* buffer);
    void gpu_buffer_write(void* buffer, const uint32_t* ptr, size_t len);
    void gpu_buffer_read(void* buffer, uint32_t* out_ptr, size_t len);
    
    void* gpu_shader_create(const char* wgsl, void** buffers, size_t buffer_count, uint64_t uniform_size);
    void gpu_shader_destroy(void* shader);
    void gpu_shader_set_uniform(void* shader, const uint8_t* ptr, size_t len);
    void gpu_shader_dispatch(void* shader, uint32_t x, uint32_t y, uint32_t z);
}

template<typename T>
class GpuBuffer {
public:
    GpuBuffer(size_t len) : len_(len) {
        // Create buffer with size in bytes
        size_t size_bytes = len * sizeof(T);
        // Round up to uint32_t alignment
        size_t size_u32 = (size_bytes + sizeof(uint32_t) - 1) / sizeof(uint32_t);
        handle_ = gpu_buffer_create(size_u32);
    }
    
    ~GpuBuffer() {
        gpu_buffer_destroy(handle_);
    }
    
    void write(const std::vector<T>& data) {
        write(data.data(), data.size());
    }
    
    void write(const T* data, size_t count) {
        size_t size_bytes = count * sizeof(T);
        size_t size_u32 = (size_bytes + sizeof(uint32_t) - 1) / sizeof(uint32_t);
        gpu_buffer_write(handle_, reinterpret_cast<const uint32_t*>(data), size_u32);
    }
    
    std::vector<T> read() {
        std::vector<T> result(len_);
        read(result.data(), len_);
        return result;
    }
    
    void read(T* out, size_t count) {
        size_t size_bytes = count * sizeof(T);
        size_t size_u32 = (size_bytes + sizeof(uint32_t) - 1) / sizeof(uint32_t);
        gpu_buffer_read(handle_, reinterpret_cast<uint32_t*>(out), size_u32);
    }
    
    void* handle() { return handle_; }
    size_t len() const { return len_; }
    size_t size_bytes() const { return len_ * sizeof(T); }
    
private:
    void* handle_;
    size_t len_;  // Number of T elements
};

// Convenience typedefs
using GpuBufferU32 = GpuBuffer<uint32_t>;
using GpuBufferF32 = GpuBuffer<float>;
using GpuBufferI32 = GpuBuffer<int32_t>;

class ComputeShader {
public:
    template<typename... Buffers>
    ComputeShader(const std::string& wgsl, std::vector<void*> buffer_handles, size_t uniform_size = 0) {
        handle_ = gpu_shader_create(
            wgsl.c_str(),
            buffer_handles.data(),
            buffer_handles.size(),
            uniform_size
        );
    }
    
    // Helper constructor that takes GpuBuffer pointers directly
    template<typename... BufferTypes>
    static ComputeShader create(const std::string& wgsl, std::initializer_list<void*> buffers, size_t uniform_size = 0) {
        return ComputeShader(wgsl, std::vector<void*>(buffers), uniform_size);
    }
    
    ~ComputeShader() {
        gpu_shader_destroy(handle_);
    }
    
    template<typename T>
    void setUniform(const T& uniform) {
        gpu_shader_set_uniform(handle_, reinterpret_cast<const uint8_t*>(&uniform), sizeof(T));
    }
    
    void dispatch(uint32_t x, uint32_t y = 1, uint32_t z = 1) {
        gpu_shader_dispatch(handle_, x, y, z);
    }
    
private:
    void* handle_;
};

// Initialize GPU once at startup
inline void initGpu() {
    gpu_init();
}

#endif