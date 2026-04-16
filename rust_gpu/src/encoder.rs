use wgpu::*;
use crate::gpu_context::GpuContext;
use crate::gpu_buffer::GpuBuffer;
use crate::staging_buffer::StagingBuffer;
use crate::compute_shader::ComputeShader;

/// Owns a wgpu CommandEncoder for one frame's worth of GPU work.
///
/// Build up work with `dispatch` and `copy_to_staging`, then call `submit`.
/// The encoder is consumed on submit and cannot be reused.
pub struct GpuEncoder {
    encoder: Option<CommandEncoder>,
}

impl GpuEncoder {
    pub fn new(ctx: &GpuContext) -> Self {
        let encoder = ctx.device.create_command_encoder(
            &CommandEncoderDescriptor { label: Some("Frame Encoder") }
        );
        Self { encoder: Some(encoder) }
    }

    /// Append a compute dispatch to this encoder. Does NOT submit.
    pub fn dispatch(&mut self, shader: &ComputeShader, x: u32, y: u32, z: u32) {
        let encoder = self.encoder.as_mut().expect("dispatch after submit");
        let mut pass = encoder.begin_compute_pass(
            &ComputePassDescriptor {
                label: None,
                timestamp_writes: None,
            }
        );
        pass.set_pipeline(&shader.pipeline);
        pass.set_bind_group(0, &shader.bind_group, &[]);
        pass.dispatch_workgroups(x, y, z);
        // pass drops here, ending the compute pass
    }

    /// Enqueue a full buffer copy from a GpuBuffer into a StagingBuffer.
    /// Types must match. Does NOT submit.
    pub fn copy_to_staging<T: bytemuck::Pod>(
        &mut self,
        src:     &GpuBuffer<T>,
        staging: &StagingBuffer<T>,
    ) {
        assert!(staging.len() >= src.len(), "copy_to_staging: staging buffer too small");
        let encoder = self.encoder.as_mut().expect("copy after submit");
        encoder.copy_buffer_to_buffer(
            src.raw(), 0,
            staging.raw(), 0,
            src.byte_size(),
        );
    }

    /// Enqueue a partial buffer copy. `elem_offset` and `elem_count` are in
    /// units of T. The copy lands at offset 0 in the staging buffer.
    /// Use this to copy only dirty chunk regions into a staging buffer sized
    /// for that region rather than the full buffer.
    pub fn copy_region_to_staging<T: bytemuck::Pod>(
        &mut self,
        src:         &GpuBuffer<T>,
        staging:     &StagingBuffer<T>,
        elem_offset: usize,
        elem_count:  usize,
    ) {
        let elem_size  = std::mem::size_of::<T>();
        let byte_offset = (elem_offset * elem_size) as u64;
        let byte_size   = (elem_count  * elem_size) as u64;

        assert!(elem_offset + elem_count <= src.len(),     "copy_region: src out of bounds");
        assert!(elem_count              <= staging.len(),  "copy_region: staging too small");

        let encoder = self.encoder.as_mut().expect("copy after submit");
        encoder.copy_buffer_to_buffer(
            src.raw(), byte_offset,
            staging.raw(), 0,
            byte_size,
        );
    }

    /// Enqueue a partial copy from GpuBuffer into a StagingBuffer at a given
    /// destination element offset.
    ///
    /// This is useful for sparse readback where the staging buffer is the
    /// same layout as the source buffer, and multiple disjoint regions need
    /// to be filled in one pass.
    pub fn copy_region_to_staging_at_offset<T: bytemuck::Pod>(
        &mut self,
        src:              &GpuBuffer<T>,
        staging:          &StagingBuffer<T>,
        src_elem_offset: usize,
        staging_elem_off: usize,
        elem_count:      usize,
    ) {
        let elem_size  = std::mem::size_of::<T>();
        let src_byte_offset      = (src_elem_offset * elem_size) as u64;
        let staging_byte_offset  = (staging_elem_off * elem_size) as u64;
        let byte_size            = (elem_count * elem_size) as u64;

        assert!(src_elem_offset + elem_count <= src.len(),          "copy_region_to_staging_at_offset: src out of bounds");
        assert!(staging_elem_off + elem_count <= staging.len(),     "copy_region_to_staging_at_offset: staging out of bounds");

        let encoder = self.encoder.as_mut().expect("copy after submit");
        encoder.copy_buffer_to_buffer(
            src.raw(), src_byte_offset,
            staging.raw(), staging_byte_offset,
            byte_size,
        );
    }

    /// Enqueue a full GPU-to-GPU buffer copy. Both buffers must have COPY_SRC/COPY_DST usage.
    /// Use this to feed one frame's output back as the next frame's input without a CPU round-trip.
    /// Does NOT submit.
    pub fn copy_buffer<T: bytemuck::Pod>(
        &mut self,
        src: &GpuBuffer<T>,
        dst: &GpuBuffer<T>,
    ) {
        assert!(dst.len() >= src.len(), "copy_buffer: dst too small");
        let encoder = self.encoder.as_mut().expect("copy_buffer after submit");
        encoder.copy_buffer_to_buffer(
            src.raw(), 0,
            dst.raw(), 0,
            src.byte_size(),
        );
    }

    /// Submit all encoded work to the GPU queue. Consumes the encoder.
    /// After this call, kick map_async on any staging buffers you want to read.
    pub fn submit(mut self, ctx: &GpuContext) {
        let encoder = self.encoder.take().expect("submit called twice");
        ctx.submit(encoder.finish());
    }
}

// Safety: if submit is not called (e.g. early return on error), just drop cleanly.
impl Drop for GpuEncoder {
    fn drop(&mut self) {
        // If encoder was not submitted, the command buffer is simply discarded.
        // wgpu handles this gracefully.
        let _ = self.encoder.take();
    }
}