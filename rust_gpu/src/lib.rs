// lib.rs - Minimal FFI that exposes primitives
use std::sync::{ OnceLock };

mod gpu_context;
mod gpu_buffer;
mod compute_shader;

use gpu_context::GpuContext;
use gpu_buffer::GpuBuffer;
use compute_shader::ComputeShader;
use wgpu::Buffer;

static CTX: OnceLock<GpuContext> = OnceLock::new();

fn ctx() -> &'static GpuContext {
    CTX.get_or_init(|| GpuContext::new())
}

// -------------------------------------------------------
// GPU context init (once)
// -------------------------------------------------------
#[no_mangle]
pub extern "C" fn gpu_init() {
    let _ = ctx(); // Initialize context
}

// -------------------------------------------------------
// Buffer creation
// -------------------------------------------------------
#[no_mangle]
pub extern "C" fn gpu_buffer_create(len: usize) -> *mut GpuBuffer<u32> {
    let buffer = GpuBuffer::<u32>::new(ctx(), Some(&vec![0; len]));
    Box::into_raw(Box::new(buffer))
}

#[no_mangle]
pub extern "C" fn gpu_buffer_destroy(buffer: *mut GpuBuffer<u32>) {
    if !buffer.is_null() {
        unsafe { drop(Box::from_raw(buffer)) };
    }
}

#[no_mangle]
pub extern "C" fn gpu_buffer_write(buffer: *mut GpuBuffer<u32>, ptr: *const u32, len: usize) {
    let buf = unsafe { &*buffer };
    let slice = unsafe { std::slice::from_raw_parts(ptr, len) };
    buf.write(ctx(), slice);
}

#[no_mangle]
pub extern "C" fn gpu_buffer_read(buffer: *mut GpuBuffer<u32>, out_ptr: *mut u32, len: usize) {
    let buf = unsafe { &*buffer };
    let data = buf.read(ctx());
    unsafe {
        std::ptr::copy_nonoverlapping(data.as_ptr(), out_ptr, len.min(data.len()));
    }
}

fn gpu_buffer_raw(buffer: *mut GpuBuffer<u32>) -> &'static Buffer {
    unsafe { (*buffer).raw() }
}

// -------------------------------------------------------
// Shader creation
// -------------------------------------------------------
#[no_mangle]
pub extern "C" fn gpu_shader_create(
    wgsl: *const std::os::raw::c_char,
    buffers: *const *mut GpuBuffer<u32>,
    buffer_count: usize,
    uniform_size: u64,
) -> *mut ComputeShader {
    let wgsl_str = unsafe { 
        std::ffi::CStr::from_ptr(wgsl).to_str().unwrap()
    };
    
    let buffer_slice = unsafe { std::slice::from_raw_parts(buffers, buffer_count) };
    let buffer_refs: Vec<&Buffer> = buffer_slice.iter()
        .map(|b| gpu_buffer_raw(*b))
        .collect();
    
    let shader = ComputeShader::new(
        ctx(),
        wgsl_str,
        &buffer_refs,
        uniform_size,
    );
    
    Box::into_raw(Box::new(shader))
}

#[no_mangle]
pub extern "C" fn gpu_shader_destroy(shader: *mut ComputeShader) {
    if !shader.is_null() {
        unsafe { drop(Box::from_raw(shader)) };
    }
}

#[no_mangle]
pub extern "C" fn gpu_shader_set_uniform(
    shader: *mut ComputeShader,
    ptr: *const u8,
    len: usize,
) {
    let s = unsafe { &*shader };
    let slice = unsafe { std::slice::from_raw_parts(ptr, len) };
    s.set_uniform(ctx(), slice);
}

#[no_mangle]
pub extern "C" fn gpu_shader_dispatch(
    shader: *mut ComputeShader,
    workgroups_x: u32,
    workgroups_y: u32,
    workgroups_z: u32,
) {
    let s = unsafe { &*shader };
    s.run(ctx(), (workgroups_x, workgroups_y, workgroups_z));
}