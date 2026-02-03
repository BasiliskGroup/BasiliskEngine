#include <basilisk/compute/gpuWrapper.hpp>

namespace bsk::internal {

template<typename... Buffers>
ComputeShader::ComputeShader(const std::string& wgsl, std::vector<void*> buffer_handles, size_t uniform_size) {
    handle_ = gpu_shader_create(
        wgsl.c_str(),
        buffer_handles.data(),
        buffer_handles.size(),
        uniform_size
    );
}

template<typename... BufferTypes>
static ComputeShader create(const std::string& wgsl, std::initializer_list<void*> buffers, size_t uniform_size) {
    return ComputeShader(wgsl, std::vector<void*>(buffers), uniform_size);
}

ComputeShader::~ComputeShader() {
    gpu_shader_destroy(handle_);
}

template<typename T>
void ComputeShader::setUniform(const T& uniform) {
    gpu_shader_set_uniform(handle_, reinterpret_cast<const uint8_t*>(&uniform), sizeof(T));
}

void ComputeShader::setUniformBytes(const uint8_t* ptr, size_t len) {
    gpu_shader_set_uniform(handle_, ptr, len);
}

void ComputeShader::dispatch(uint32_t x, uint32_t y, uint32_t z) {
    gpu_shader_dispatch(handle_, x, y, z);
}

}