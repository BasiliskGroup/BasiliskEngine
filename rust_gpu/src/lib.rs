// lib.rs — FFI surface for GPU compute primitives.
//
// Design rules:
//   - Every type is heap-allocated and returned as an opaque *mut T.
//   - Caller owns all pointers and must call the corresponding _destroy.
//   - No blocking happens unless the function name contains "wait" or "collect".
//   - Encoder functions only encode work; nothing runs until gpu_encoder_submit.

use std::sync::OnceLock;
use wgpu::Buffer;

mod gpu_context;
mod gpu_buffer;
mod staging_buffer;
mod compute_shader;
mod encoder;

use gpu_context::GpuContext;
use gpu_buffer::GpuBuffer;
use staging_buffer::StagingBuffer;
use compute_shader::ComputeShader;
use encoder::GpuEncoder;

// ---------------------------------------------------------------------------
// Global context
// ---------------------------------------------------------------------------

static CTX: OnceLock<GpuContext> = OnceLock::new();

fn ctx() -> &'static GpuContext {
    CTX.get_or_init(GpuContext::new)
}

#[no_mangle]
pub extern "C" fn gpu_init() {
    let _ = ctx();
}

// ---------------------------------------------------------------------------
// GpuBuffer  (GPU-side storage, writable from CPU via queue)
// ---------------------------------------------------------------------------

#[no_mangle]
pub extern "C" fn gpu_buffer_create(len: usize) -> *mut GpuBuffer<u32> {
    Box::into_raw(Box::new(GpuBuffer::<u32>::new(ctx(), len)))
}

#[no_mangle]
pub extern "C" fn gpu_buffer_destroy(ptr: *mut GpuBuffer<u32>) {
    if !ptr.is_null() { unsafe { drop(Box::from_raw(ptr)) }; }
}

/// Upload the full buffer. Enqueued to the wgpu queue immediately (no submit needed).
#[no_mangle]
pub extern "C" fn gpu_buffer_write(ptr: *mut GpuBuffer<u32>, data: *const u32, len: usize) {
    let buf   = unsafe { &*ptr };
    let slice = unsafe { std::slice::from_raw_parts(data, len) };
    buf.write(ctx(), slice);
}

/// Upload a sub-region. elem_offset and len are element counts (u32), not bytes.
#[no_mangle]
pub extern "C" fn gpu_buffer_write_region(
    ptr:         *mut GpuBuffer<u32>,
    elem_offset: usize,
    data:        *const u32,
    len:         usize,
) {
    let buf   = unsafe { &*ptr };
    let slice = unsafe { std::slice::from_raw_parts(data, len) };
    buf.write_region(ctx(), elem_offset, slice);
}

/// Zero the entire buffer (convenience — avoids allocating a zeroed vec in C++).
#[no_mangle]
pub extern "C" fn gpu_buffer_zero(ptr: *mut GpuBuffer<u32>) {
    unsafe { &*ptr }.zero(ctx());
}

// ---------------------------------------------------------------------------
// StagingBuffer  (CPU-readable, filled by encoder copy commands)
// ---------------------------------------------------------------------------

#[no_mangle]
pub extern "C" fn gpu_staging_create(len: usize) -> *mut StagingBuffer<u32> {
    Box::into_raw(Box::new(StagingBuffer::<u32>::new(ctx(), len)))
}

#[no_mangle]
pub extern "C" fn gpu_staging_destroy(ptr: *mut StagingBuffer<u32>) {
    if !ptr.is_null() { unsafe { drop(Box::from_raw(ptr)) }; }
}

/// Kick off async mapping of the staging buffer. Call this immediately after
/// gpu_encoder_submit. Does NOT block.
#[no_mangle]
pub extern "C" fn gpu_staging_map_async(ptr: *mut StagingBuffer<u32>) {
    unsafe { &*ptr }.map_async();
}

/// Block until the map is ready, copy the full buffer into out, then unmap.
/// out must point to at least `len` u32 elements.
#[no_mangle]
pub extern "C" fn gpu_staging_collect(
    ptr: *mut StagingBuffer<u32>,
    out: *mut u32,
    len: usize,
) {
    let staging = unsafe { &*ptr };
    let slice   = unsafe { std::slice::from_raw_parts_mut(out, len) };
    staging.collect(ctx(), slice);
}

/// Block and read back only a sub-region of the staging buffer.
/// elem_offset and len are element counts (u32).
/// The region is written to out starting at index 0.
#[no_mangle]
pub extern "C" fn gpu_staging_collect_region(
    ptr:         *mut StagingBuffer<u32>,
    out:         *mut u32,
    elem_offset: usize,
    len:         usize,
) {
    let staging = unsafe { &*ptr };
    let slice   = unsafe { std::slice::from_raw_parts_mut(out, len) };
    staging.collect_region(ctx(), slice, elem_offset, len);
}

// ---------------------------------------------------------------------------
// GpuEncoder  (one per frame — batch all dispatches and copies, submit once)
// ---------------------------------------------------------------------------

#[no_mangle]
pub extern "C" fn gpu_encoder_create() -> *mut GpuEncoder {
    Box::into_raw(Box::new(GpuEncoder::new(ctx())))
}

#[no_mangle]
pub extern "C" fn gpu_encoder_destroy(ptr: *mut GpuEncoder) {
    if !ptr.is_null() { unsafe { drop(Box::from_raw(ptr)) }; }
}

/// Append a compute dispatch to the encoder. Does NOT submit.
#[no_mangle]
pub extern "C" fn gpu_encoder_dispatch(
    enc:    *mut GpuEncoder,
    shader: *mut ComputeShader,
    x: u32, y: u32, z: u32,
) {
    let enc    = unsafe { &mut *enc };
    let shader = unsafe { &*shader };
    enc.dispatch(shader, x, y, z);
}

/// Enqueue a GPU-to-GPU buffer copy (no CPU involvement).
/// Use this to copy cellsB -> cellsA each frame instead of pointer-swapping.
/// Does NOT submit.
#[no_mangle]
pub extern "C" fn gpu_encoder_copy_buffer(
    enc: *mut GpuEncoder,
    src: *mut GpuBuffer<u32>,
    dst: *mut GpuBuffer<u32>,
) {
    let enc = unsafe { &mut *enc };
    let src = unsafe { &*src };
    let dst = unsafe { &*dst };
    enc.copy_buffer(src, dst);
}

/// Enqueue a full buffer copy from a GpuBuffer into a StagingBuffer.
/// Does NOT submit.
#[no_mangle]
pub extern "C" fn gpu_encoder_copy_to_staging(
    enc:     *mut GpuEncoder,
    src:     *mut GpuBuffer<u32>,
    staging: *mut StagingBuffer<u32>,
) {
    let enc     = unsafe { &mut *enc };
    let src     = unsafe { &*src };
    let staging = unsafe { &*staging };
    enc.copy_to_staging(src, staging);
}

/// Enqueue a partial copy from GpuBuffer into StagingBuffer.
/// elem_offset is the start element in the source buffer.
/// elem_count elements are copied; they land at offset 0 in the staging buffer.
/// Does NOT submit.
#[no_mangle]
pub extern "C" fn gpu_encoder_copy_region_to_staging(
    enc:         *mut GpuEncoder,
    src:         *mut GpuBuffer<u32>,
    staging:     *mut StagingBuffer<u32>,
    elem_offset: usize,
    elem_count:  usize,
) {
    let enc     = unsafe { &mut *enc };
    let src     = unsafe { &*src };
    let staging = unsafe { &*staging };
    enc.copy_region_to_staging(src, staging, elem_offset, elem_count);
}

/// Enqueue a partial copy from GpuBuffer into StagingBuffer at a specific
/// destination element offset.
///
/// Useful for sparse readback where the staging buffer uses the same
/// indexing/layout as the source buffer.
#[no_mangle]
pub extern "C" fn gpu_encoder_copy_region_to_staging_at_offset(
    enc:              *mut GpuEncoder,
    src:              *mut GpuBuffer<u32>,
    staging:          *mut StagingBuffer<u32>,
    elem_offset:     usize,
    staging_offset:  usize,
    elem_count:      usize,
) {
    let enc     = unsafe { &mut *enc };
    let src     = unsafe { &*src };
    let staging = unsafe { &*staging };
    enc.copy_region_to_staging_at_offset(src, staging, elem_offset, staging_offset, elem_count);
}

/// Submit all encoded work in one queue submission. Destroys the encoder.
/// After this, call gpu_staging_map_async on any staging buffers you want to read.
#[no_mangle]
pub extern "C" fn gpu_encoder_submit(ptr: *mut GpuEncoder) {
    // Take ownership out of the raw pointer and let submit() consume it.
    let enc = unsafe { Box::from_raw(ptr) };
    enc.submit(ctx());
    // Box is consumed by submit (which takes self), so no double-free.
}

// ---------------------------------------------------------------------------
// ComputeShader
// ---------------------------------------------------------------------------

fn gpu_buffer_raw(ptr: *mut GpuBuffer<u32>) -> &'static Buffer {
    unsafe { (*ptr).raw() }
}

#[no_mangle]
pub extern "C" fn gpu_shader_create(
    wgsl:         *const std::os::raw::c_char,
    buffers:      *const *mut GpuBuffer<u32>,
    buffer_count: usize,
    uniform_size: u64,
) -> *mut ComputeShader {
    let wgsl_str     = unsafe { std::ffi::CStr::from_ptr(wgsl).to_str().unwrap() };
    let buffer_slice = unsafe { std::slice::from_raw_parts(buffers, buffer_count) };
    let buffer_refs: Vec<&Buffer> = buffer_slice.iter()
        .map(|b| gpu_buffer_raw(*b))
        .collect();

    Box::into_raw(Box::new(ComputeShader::new(ctx(), wgsl_str, &buffer_refs, uniform_size)))
}

#[no_mangle]
pub extern "C" fn gpu_shader_destroy(ptr: *mut ComputeShader) {
    if !ptr.is_null() { unsafe { drop(Box::from_raw(ptr)) }; }
}

/// Write uniform data. Safe to call any time before the encoder dispatching
/// this shader is submitted.
#[no_mangle]
pub extern "C" fn gpu_shader_set_uniform(
    ptr:  *mut ComputeShader,
    data: *const u8,
    len:  usize,
) {
    let shader = unsafe { &*ptr };
    let slice  = unsafe { std::slice::from_raw_parts(data, len) };
    shader.set_uniform(ctx(), slice);
}

// ---------------------------------------------------------------------------
// Device polling (rarely needed directly — collect functions call poll_wait)
// ---------------------------------------------------------------------------

/// Non-blocking poll. Process any completed GPU work.
#[no_mangle]
pub extern "C" fn gpu_poll() {
    ctx().poll();
}

/// Blocking poll. Stall CPU until all submitted GPU work completes.
/// Prefer calling gpu_staging_collect which calls this internally.
#[no_mangle]
pub extern "C" fn gpu_poll_wait() {
    ctx().poll_wait();
}