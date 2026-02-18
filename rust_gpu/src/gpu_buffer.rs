use wgpu::*;
use std::marker::PhantomData;
use crate::gpu_context::GpuContext;

pub struct GpuBuffer<T: bytemuck::Pod> {
    buffer: Buffer,
    readback: Buffer,
    len: usize,
    _marker: PhantomData<T>
}

impl<T: bytemuck::Pod> GpuBuffer<T> {
    pub fn new(ctx: &GpuContext, data: Option<&[T]>) -> Self {
        let size = (data.map(|d| d.len()).unwrap_or(1) * std::mem::size_of::<T>()) as u64;

        let buffer = ctx.device.create_buffer(&BufferDescriptor {
            label: Some("Storage Buffer"),
            size,
            usage: BufferUsages::STORAGE
                 | BufferUsages::COPY_SRC
                 | BufferUsages::COPY_DST,
            mapped_at_creation: false
        });

        let readback = ctx.device.create_buffer(&BufferDescriptor {
            label: Some("Readback Buffer"),
            size,
            usage: BufferUsages::MAP_READ | BufferUsages::COPY_DST,
            mapped_at_creation: false
        });

        if let Some(init) = data {
            ctx.queue.write_buffer(&buffer, 0, bytemuck::cast_slice(init));
        }

        Self {
            buffer,
            readback,
            len: data.map(|d| d.len()).unwrap_or(0),
            _marker: PhantomData
        }
    }

    pub fn write(&self, ctx: &GpuContext, data: &[T]) {
        ctx.queue.write_buffer(&self.buffer, 0, bytemuck::cast_slice(data));
    }

    pub fn read(&self, ctx: &GpuContext) -> Vec<T> {
        let mut encoder = ctx.device.create_command_encoder(&CommandEncoderDescriptor { 
            label: Some("Readback Encoder") 
        });

        encoder.copy_buffer_to_buffer(
            &self.buffer,
            0,
            &self.readback,
            0,
            (self.len * std::mem::size_of::<T>()) as u64,
        );

        ctx.submit(encoder.finish());

        let slice = self.readback.slice(..);
        slice.map_async(MapMode::Read, |_| {});
        ctx.poll(true);

        let data = slice.get_mapped_range();
        let result = bytemuck::cast_slice(&data).to_vec();

        drop(data);
        self.readback.unmap();

        result
    }

    pub fn raw(&self) -> &Buffer {
        &self.buffer
    }

    pub fn len(&self) -> usize {
        self.len
    }
}