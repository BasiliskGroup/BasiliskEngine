#include <basilisk/compute/gpuWrapper.hpp>

namespace bsk::internal {

template<typename T>
GpuBuffer<T>::GpuBuffer(size_t len) : len_(len) {
    // Create buffer with size in bytes
    size_t size_bytes = len * sizeof(T);
    // Round up to uint32_t alignment
    size_t size_u32 = (size_bytes + sizeof(uint32_t) - 1) / sizeof(uint32_t);
    handle_ = gpu_buffer_create(size_u32);
}

template<typename T>
GpuBuffer<T>::~GpuBuffer() {
    gpu_buffer_destroy(handle_);
}

template<typename T>
void GpuBuffer<T>::write(const std::vector<T>& data) {
    write(data.data(), data.size());
}

template<typename T>
void GpuBuffer<T>::write(const T* data, size_t count) {
    size_t size_bytes = count * sizeof(T);
    size_t size_u32 = (size_bytes + sizeof(uint32_t) - 1) / sizeof(uint32_t);
    gpu_buffer_write(handle_, reinterpret_cast<const uint32_t*>(data), size_u32);
}

template<typename T>
std::vector<T> GpuBuffer<T>::read() {
    std::vector<T> result(len_);
    read(result.data(), len_);
    return result;
}

template<typename T>
void GpuBuffer<T>::read(T* out, size_t count) {
    size_t size_bytes = count * sizeof(T);
    size_t size_u32 = (size_bytes + sizeof(uint32_t) - 1) / sizeof(uint32_t);
    gpu_buffer_read(handle_, reinterpret_cast<uint32_t*>(out), size_u32);
}
}