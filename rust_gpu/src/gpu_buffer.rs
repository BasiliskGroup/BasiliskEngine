use wgpu::*;
use std::marker::PhantomData;
use crate::gpu_context::GpuContext;

/// GPU-side storage buffer. Readable by shaders, writable from CPU via queue.
/// Has no staging buffer embedded — use StagingBuffer explicitly when readback is needed.
pub struct GpuBuffer<T: bytemuck::Pod> {
    pub(crate) buffer: Buffer,
    len:               usize,
    _marker:           PhantomData<T>,
}

impl<T: bytemuck::Pod> GpuBuffer<T> {
    pub fn new(ctx: &GpuContext, len: usize) -> Self {
        let size = (len * std::mem::size_of::<T>()) as u64;

        let buffer = ctx.device.create_buffer(&BufferDescriptor {
            label:              Some("Storage Buffer"),
            size,
            usage:              BufferUsages::STORAGE
                              | BufferUsages::COPY_SRC
                              | BufferUsages::COPY_DST,
            mapped_at_creation: false,
        });

        Self { buffer, len, _marker: PhantomData }
    }

    /// Upload the entire buffer from CPU memory. Enqueued immediately via the wgpu queue
    /// (internally staged by wgpu; no explicit submit needed).
    pub fn write(&self, ctx: &GpuContext, data: &[T]) {
        assert!(data.len() <= self.len, "write: data longer than buffer");
        ctx.queue.write_buffer(&self.buffer, 0, bytemuck::cast_slice(data));
    }

    /// Upload a sub-region. `elem_offset` and `data.len()` are in units of T.
    pub fn write_region(&self, ctx: &GpuContext, elem_offset: usize, data: &[T]) {
        assert!(elem_offset + data.len() <= self.len, "write_region: out of bounds");
        let byte_offset = (elem_offset * std::mem::size_of::<T>()) as u64;
        ctx.queue.write_buffer(&self.buffer, byte_offset, bytemuck::cast_slice(data));
    }

    /// Zero the entire buffer by uploading a zeroed vec.
    pub fn zero(&self, ctx: &GpuContext) {
        let zeros = vec![0u8; self.len * std::mem::size_of::<T>()];
        ctx.queue.write_buffer(&self.buffer, 0, &zeros);
    }

    pub fn raw(&self) -> &Buffer { &self.buffer }
    pub fn len(&self) -> usize   { self.len }
    pub fn byte_size(&self) -> u64 { (self.len * std::mem::size_of::<T>()) as u64 }
}
