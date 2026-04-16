#pragma once
#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>
#include <cassert>

namespace bsk::internal {

// ---------------------------------------------------------------------------
// Rust FFI declarations
// ---------------------------------------------------------------------------
extern "C" {
    // Context
    void gpu_init();
    void gpu_poll();
    void gpu_poll_wait();

    // GpuBuffer — GPU-side storage, writable from CPU
    void*  gpu_buffer_create(size_t len);
    void   gpu_buffer_destroy(void* buffer);
    void   gpu_buffer_write(void* buffer, const uint32_t* data, size_t len);
    void   gpu_buffer_write_region(void* buffer, size_t elem_offset, const uint32_t* data, size_t len);
    void   gpu_buffer_zero(void* buffer);

    // StagingBuffer — CPU-readable, filled by encoder copy commands
    void*  gpu_staging_create(size_t len);
    void   gpu_staging_destroy(void* staging);
    void   gpu_staging_map_async(void* staging);
    void   gpu_staging_collect(void* staging, uint32_t* out, size_t len);
    void   gpu_staging_collect_region(void* staging, uint32_t* out, size_t elem_offset, size_t len);

    // GpuEncoder — one per frame, batch all work, submit once
    void*  gpu_encoder_create();
    void   gpu_encoder_destroy(void* encoder);
    void   gpu_encoder_dispatch(void* encoder, void* shader, uint32_t x, uint32_t y, uint32_t z);
    void   gpu_encoder_copy_buffer(void* encoder, void* src, void* dst);
    void   gpu_encoder_copy_to_staging(void* encoder, void* src, void* staging);
    void   gpu_encoder_copy_region_to_staging(void* encoder, void* src, void* staging,
                                               size_t elem_offset, size_t elem_count);
    void   gpu_encoder_copy_region_to_staging_at_offset(void* encoder, void* src, void* staging,
                                                         size_t elem_offset, size_t staging_offset, size_t elem_count);
    void   gpu_encoder_submit(void* encoder); // consumes encoder — do not call destroy after

    // ComputeShader
    void*  gpu_shader_create(const char* wgsl, void** buffers, size_t buffer_count, uint64_t uniform_size);
    void   gpu_shader_destroy(void* shader);
    void   gpu_shader_set_uniform(void* shader, const uint8_t* ptr, size_t len);
}

// ---------------------------------------------------------------------------
// GpuBuffer<T>
// GPU-side storage buffer. No readback — use StagingBuffer for that.
// ---------------------------------------------------------------------------
template<typename T>
class GpuBuffer {
public:
    explicit GpuBuffer(size_t len) : len_(len) {
        // Rust layer takes element count in u32 units; round up for non-u32 T
        size_t u32_count = (len * sizeof(T) + sizeof(uint32_t) - 1) / sizeof(uint32_t);
        handle_ = gpu_buffer_create(u32_count);
    }

    ~GpuBuffer() { gpu_buffer_destroy(handle_); }

    GpuBuffer(const GpuBuffer&)            = delete;
    GpuBuffer& operator=(const GpuBuffer&) = delete;

    // Upload full buffer
    void write(const T* data, size_t count) {
        assert(count <= len_);
        size_t u32_count = (count * sizeof(T) + sizeof(uint32_t) - 1) / sizeof(uint32_t);
        gpu_buffer_write(handle_, reinterpret_cast<const uint32_t*>(data), u32_count);
    }

    void write(const std::vector<T>& data) { write(data.data(), data.size()); }

    // Upload sub-region. elem_offset and data.size() in units of T.
    void writeRegion(size_t elem_offset, const T* data, size_t count) {
        assert(elem_offset + count <= len_);
        size_t offset_u32 = elem_offset * sizeof(T) / sizeof(uint32_t);
        size_t count_u32  = (count * sizeof(T) + sizeof(uint32_t) - 1) / sizeof(uint32_t);
        gpu_buffer_write_region(handle_, offset_u32, reinterpret_cast<const uint32_t*>(data), count_u32);
    }

    // Zero the entire buffer on the GPU
    void zero() { gpu_buffer_zero(handle_); }

    void*  handle() const { return handle_; }
    size_t len()    const { return len_; }
    size_t sizeBytes() const { return len_ * sizeof(T); }

private:
    void*  handle_;
    size_t len_;
};

using GpuBufferU32 = GpuBuffer<uint32_t>;
using GpuBufferF32 = GpuBuffer<float>;
using GpuBufferI32 = GpuBuffer<int32_t>;

// Legacy compatibility for bindings and existing call sites.
enum class GpuBufferDtype { U32, F32, I32 };

// ---------------------------------------------------------------------------
// StagingBuffer<T>
// CPU-readable buffer populated by GpuEncoder copy commands.
//
// Usage per frame:
//   encoder.copyToStaging(src, *this)   — enqueue copy
//   encoder.submit()                    — submit all work
//   mapAsync()                          — kick async map (non-blocking)
//   -- other CPU work --
//   collect(out, len)                   — block + read + unmap
// ---------------------------------------------------------------------------
template<typename T>
class StagingBuffer {
public:
    explicit StagingBuffer(size_t len) : len_(len) {
        size_t u32_count = (len * sizeof(T) + sizeof(uint32_t) - 1) / sizeof(uint32_t);
        handle_ = gpu_staging_create(u32_count);
    }

    ~StagingBuffer() { gpu_staging_destroy(handle_); }

    StagingBuffer(const StagingBuffer&)            = delete;
    StagingBuffer& operator=(const StagingBuffer&) = delete;

    // Kick async map after encoder submit. Non-blocking.
    void mapAsync() { gpu_staging_map_async(handle_); }

    // Block, read full buffer into out, unmap.
    void collect(T* out, size_t count) {
        assert(count <= len_);
        size_t u32_count = (count * sizeof(T) + sizeof(uint32_t) - 1) / sizeof(uint32_t);
        gpu_staging_collect(handle_, reinterpret_cast<uint32_t*>(out), u32_count);
    }

    void collect(std::vector<T>& out) {
        out.resize(len_);
        collect(out.data(), len_);
    }

    // Block, read sub-region starting at elem_offset into out[0..len), unmap.
    void collectRegion(T* out, size_t elem_offset, size_t len) {
        assert(elem_offset + len <= len_);
        size_t offset_u32 = elem_offset * sizeof(T) / sizeof(uint32_t);
        size_t len_u32    = (len * sizeof(T) + sizeof(uint32_t) - 1) / sizeof(uint32_t);
        gpu_staging_collect_region(handle_, reinterpret_cast<uint32_t*>(out), offset_u32, len_u32);
    }

    void*  handle() const { return handle_; }
    size_t len()    const { return len_; }

private:
    void*  handle_;
    size_t len_;
};

using StagingBufferU32 = StagingBuffer<uint32_t>;

// ---------------------------------------------------------------------------
// GpuEncoder
// One per frame. Accumulates dispatches and copies, submits in one call.
// submit() consumes the underlying encoder — the destructor is a no-op after.
// ---------------------------------------------------------------------------
class GpuEncoder {
public:
    GpuEncoder() : handle_(gpu_encoder_create()) {}

    ~GpuEncoder() {
        // If submit() was not called (e.g. early return), destroy cleanly.
        if (handle_) gpu_encoder_destroy(handle_);
    }

    GpuEncoder(const GpuEncoder&)            = delete;
    GpuEncoder& operator=(const GpuEncoder&) = delete;

    // Append a compute dispatch. Does NOT submit.
    void dispatch(void* shaderHandle, uint32_t x, uint32_t y, uint32_t z = 1) {
        assert(handle_ && "dispatch after submit");
        gpu_encoder_dispatch(handle_, shaderHandle, x, y, z);
    }

    // GPU-to-GPU buffer copy — no CPU involvement. Does NOT submit.
    // Use this to feed cellsB back into cellsA each frame.
    template<typename T>
    void copyBuffer(const GpuBuffer<T>& src, GpuBuffer<T>& dst) {
        assert(handle_ && "copyBuffer after submit");
        gpu_encoder_copy_buffer(handle_, src.handle(), dst.handle());
    }

    // Full buffer copy into staging. Does NOT submit.
    template<typename T>
    void copyToStaging(const GpuBuffer<T>& src, StagingBuffer<T>& staging) {
        assert(handle_ && "copy after submit");
        gpu_encoder_copy_to_staging(handle_, src.handle(), staging.handle());
    }

    // Partial copy: elem_count elements starting at elem_offset in src,
    // landing at offset 0 in staging. Does NOT submit.
    template<typename T>
    void copyRegionToStaging(const GpuBuffer<T>& src, StagingBuffer<T>& staging,
                              size_t elem_offset, size_t elem_count) {
        assert(handle_ && "copy after submit");
        gpu_encoder_copy_region_to_staging(handle_, src.handle(), staging.handle(),
                                           elem_offset, elem_count);
    }

    // Partial copy landing at a specific element offset inside staging.
    template<typename T>
    void copyRegionToStagingAtOffset(const GpuBuffer<T>& src, StagingBuffer<T>& staging,
                                     size_t elem_offset, size_t staging_offset,
                                     size_t elem_count) {
        assert(handle_ && "copy after submit");
        gpu_encoder_copy_region_to_staging_at_offset(handle_, src.handle(), staging.handle(),
                                                     elem_offset, staging_offset, elem_count);
    }

    // Submit all encoded work. Consumes the encoder handle — do not use after.
    void submit() {
        assert(handle_ && "submit called twice");
        gpu_encoder_submit(handle_); // Rust takes ownership and frees
        handle_ = nullptr;
    }

private:
    void* handle_;
};

// ---------------------------------------------------------------------------
// ComputeShader
// Owns a pipeline + bind group. Does not dispatch itself — use GpuEncoder.
// ---------------------------------------------------------------------------
class ComputeShader {
public:
    ComputeShader(const std::string& wgsl, std::vector<void*> bufferHandles,
                  size_t uniformSize = 0) {
        handle_ = gpu_shader_create(
            wgsl.c_str(),
            bufferHandles.data(),
            bufferHandles.size(),
            static_cast<uint64_t>(uniformSize)
        );
    }

    ~ComputeShader() { gpu_shader_destroy(handle_); }

    ComputeShader(const ComputeShader&)            = delete;
    ComputeShader& operator=(const ComputeShader&) = delete;

    template<typename T>
    void setUniform(const T& data) {
        gpu_shader_set_uniform(handle_,
            reinterpret_cast<const uint8_t*>(&data), sizeof(T));
    }

    // Legacy compatibility: raw bytes uniform setter.
    void setUniformBytes(const uint8_t* ptr, size_t len) {
        gpu_shader_set_uniform(handle_, ptr, len);
    }

    // Legacy compatibility: immediate dispatch via one-shot encoder.
    void dispatch(uint32_t x, uint32_t y = 1, uint32_t z = 1) {
        GpuEncoder encoder;
        encoder.dispatch(handle_, x, y, z);
        encoder.submit();
    }

    void* handle() const { return handle_; }

private:
    void* handle_;
};

// Legacy untyped buffer shim used by Python bindings.
class GpuBufferAny {
public:
    explicit GpuBufferAny(GpuBufferDtype dtype, size_t element_count)
        : dtype_(dtype), element_count_(element_count), element_size_(sizeof(uint32_t)) {
        handle_ = gpu_buffer_create(element_count_);
    }

    ~GpuBufferAny() {
        gpu_buffer_destroy(handle_);
    }

    void writeBytes(const uint8_t* ptr, size_t byte_count) {
        const size_t u32_count = (byte_count + sizeof(uint32_t) - 1) / sizeof(uint32_t);
        gpu_buffer_write(handle_, reinterpret_cast<const uint32_t*>(ptr), u32_count);
    }

    void readBytes(uint8_t* out, size_t byte_count) const {
        const size_t u32_count = (byte_count + sizeof(uint32_t) - 1) / sizeof(uint32_t);
        StagingBuffer<uint32_t> staging(u32_count);
        void* encoder = gpu_encoder_create();
        gpu_encoder_copy_to_staging(encoder, handle_, staging.handle());
        gpu_encoder_submit(encoder);
        staging.collect(reinterpret_cast<uint32_t*>(out), u32_count);
    }

    void* handle() const { return handle_; }
    size_t len() const { return element_count_; }
    size_t element_size() const { return element_size_; }
    size_t size_bytes() const { return element_count_ * element_size_; }
    GpuBufferDtype dtype() const { return dtype_; }

private:
    void* handle_ = nullptr;
    size_t element_count_ = 0;
    size_t element_size_ = 0;
    GpuBufferDtype dtype_ = GpuBufferDtype::U32;
};

// ---------------------------------------------------------------------------
// Init
// ---------------------------------------------------------------------------
inline void initGpu() { gpu_init(); }

} // namespace bsk::internal