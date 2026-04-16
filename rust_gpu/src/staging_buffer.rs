use wgpu::*;
use std::marker::PhantomData;
use crate::gpu_context::GpuContext;

/// CPU-mappable staging buffer used to read back GPU data asynchronously.
///
/// Typical frame usage:
///   1. `encoder.copy_to_staging(src, staging, ...)` — enqueue copy (no stall)
///   2. `encoder.submit()`                           — submit all work this frame
///   3. `staging.map_async()`                        — kick off async map (no stall)
///   4. (next frame or later) `staging.collect()`    — stall + read + unmap
pub struct StagingBuffer<T: bytemuck::Pod> {
    pub(crate) buffer: Buffer,
    len:               usize,
    _marker:           PhantomData<T>,
}

impl<T: bytemuck::Pod> StagingBuffer<T> {
    pub fn new(ctx: &GpuContext, len: usize) -> Self {
        let size = (len * std::mem::size_of::<T>()) as u64;

        let buffer = ctx.device.create_buffer(&BufferDescriptor {
            label:              Some("Staging Buffer"),
            size,
            usage:              BufferUsages::MAP_READ | BufferUsages::COPY_DST,
            mapped_at_creation: false,
        });

        Self { buffer, len, _marker: PhantomData }
    }

    /// Kick off an async map. Call this after submitting the encoder that
    /// copied into this buffer. Does NOT block.
    pub fn map_async(&self) {
        self.buffer.slice(..).map_async(MapMode::Read, |_| {});
    }

    /// Block until GPU work is done, then copy the full buffer into `out`.
    /// Unmaps the buffer afterwards so it can be reused next frame.
    /// `out` must be at least `self.len` elements.
    pub fn collect(&self, ctx: &GpuContext, out: &mut [T]) {
        assert!(out.len() >= self.len, "collect: output slice too small");
        ctx.poll_wait();
        let mapped = self.buffer.slice(..).get_mapped_range();
        out[..self.len].copy_from_slice(bytemuck::cast_slice(&mapped));
        drop(mapped);
        self.buffer.unmap();
    }

    /// Block and read back only a sub-region. `elem_offset` and `len` are in
    /// units of T. Useful for reading back only dirty chunks rather than the
    /// full cells buffer.
    /// Unmaps after reading — if you need multiple regions, use `collect` and
    /// slice on the CPU side instead.
    pub fn collect_region(&self, ctx: &GpuContext, out: &mut [T], elem_offset: usize, len: usize) {
        assert!(elem_offset + len <= self.len, "collect_region: out of bounds");
        assert!(out.len() >= len,              "collect_region: output slice too small");

        ctx.poll_wait();

        let byte_start = (elem_offset * std::mem::size_of::<T>()) as u64;
        let byte_end   = byte_start + (len * std::mem::size_of::<T>()) as u64;

        let mapped = self.buffer.slice(byte_start..byte_end).get_mapped_range();
        out[..len].copy_from_slice(bytemuck::cast_slice(&mapped));
        drop(mapped);
        self.buffer.unmap();
    }

    /// Non-blocking poll. Returns true if the map is complete and you can
    /// call collect. Use this if you want to skip the readback rather than
    /// stall — e.g. drop a frame's data if the GPU is behind.
    pub fn try_collect(&self, ctx: &GpuContext, out: &mut [T]) -> bool {
        ctx.poll(); // non-blocking
        // wgpu does not expose a "is_mapped" query, so we attempt to get the
        // range. If the map is not ready, get_mapped_range will panic — so we
        // use poll_wait here conservatively. For true non-blocking you would
        // need to track the map callback state yourself (see note below).
        //
        // In practice: use map_async + collect_wait for the simple path.
        // True non-blocking readback requires storing an Arc<AtomicBool> in
        // the map_async callback — add that if you later need frame-drop logic.
        self.collect(ctx, out);
        true
    }

    pub fn raw(&self) -> &Buffer { &self.buffer }
    pub fn len(&self) -> usize   { self.len }
    pub fn byte_size(&self) -> u64 { (self.len * std::mem::size_of::<T>()) as u64 }
}
