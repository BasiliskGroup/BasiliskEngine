use wgpu::*;
use crate::gpu_context::GpuContext;

pub struct ComputeShader {
    pipeline: ComputePipeline,
    bind_group: BindGroup,
    uniform_buffer: Option<Buffer>,
}

impl ComputeShader {
    pub fn new(
        ctx: &GpuContext,
        wgsl: &str,
        buffers: &[&Buffer],
        uniform_size: u64,
    ) -> Self {
        let shader = ctx.device.create_shader_module(
            ShaderModuleDescriptor {
                label: None,
                source: ShaderSource::Wgsl(wgsl.into()),
            },
        );

        let has_uniform = uniform_size > 0;
        let buffer_start_binding = if has_uniform { 1 } else { 0 };
        
        let mut entries = Vec::new();
        
        // Add uniform at binding 0 if needed
        if has_uniform {
            entries.push(BindGroupLayoutEntry {
                binding: 0,
                visibility: ShaderStages::COMPUTE,
                ty: BindingType::Buffer {
                    ty: BufferBindingType::Uniform,
                    has_dynamic_offset: false,
                    min_binding_size: None,
                },
                count: None,
            });
        }

        // Add storage buffers
        entries.extend(buffers.iter().enumerate().map(|(i, _)| {
            BindGroupLayoutEntry {
                binding: (buffer_start_binding + i) as u32,
                visibility: ShaderStages::COMPUTE,
                ty: BindingType::Buffer {
                    ty: BufferBindingType::Storage { read_only: false },
                    has_dynamic_offset: false,
                    min_binding_size: None,
                },
                count: None,
            }
        }));

        let layout = ctx.device.create_bind_group_layout(
            &BindGroupLayoutDescriptor {
                label: None,
                entries: &entries,
            }
        );

        let pipeline_layout = ctx.device.create_pipeline_layout(
            &PipelineLayoutDescriptor {
                label: None,
                bind_group_layouts: &[&layout],
                push_constant_ranges: &[],
            }
        );

        let pipeline = ctx.device.create_compute_pipeline(
            &ComputePipelineDescriptor {
                label: None,
                layout: Some(&pipeline_layout),
                module: &shader,
                entry_point: "main",
            }
        );

        let uniform_buffer = if uniform_size > 0 {
            Some(ctx.device.create_buffer(&BufferDescriptor {
                label: Some("Uniform Buffer"),
                size: uniform_size,
                usage: BufferUsages::UNIFORM | BufferUsages::COPY_DST,
                mapped_at_creation: false,
            }))
        } else {
            None
        };

        let mut bg_entries = Vec::new();
        
        // Add uniform buffer entry if it exists
        if let Some(ref ub) = uniform_buffer {
            bg_entries.push(BindGroupEntry {
                binding: 0,
                resource: ub.as_entire_binding(),
            });
        }

        // Add storage buffer entries
        bg_entries.extend(buffers.iter().enumerate().map(|(i, b)| {
            BindGroupEntry {
                binding: (buffer_start_binding + i) as u32,
                resource: b.as_entire_binding(),
            }
        }));

        let bind_group = ctx.device.create_bind_group(
            &BindGroupDescriptor {
                label: None,
                layout: &layout,
                entries: &bg_entries,
            }
        );

        Self { pipeline, bind_group, uniform_buffer }
    }

    pub fn set_uniform(&self, ctx: &GpuContext, data: &[u8]) {
        if let Some(ref buffer) = self.uniform_buffer {
            ctx.queue.write_buffer(buffer, 0, data);
        }
    }

    pub fn run(&self, ctx: &GpuContext, workgroups: (u32, u32, u32)) {
        let mut encoder = ctx.device.create_command_encoder(
            &CommandEncoderDescriptor { label: None }
        );

        {
            let mut pass = encoder.begin_compute_pass(
                &ComputePassDescriptor { label: None }
            );
            pass.set_pipeline(&self.pipeline);
            pass.set_bind_group(0, &self.bind_group, &[]);
            pass.dispatch_workgroups(workgroups.0, workgroups.1, workgroups.2);
        }

        ctx.submit(encoder.finish());
    }
}