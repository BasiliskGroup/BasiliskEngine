use wgpu::*;
use crate::gpu_context::GpuContext;

fn parse_storage_read_only_map(wgsl: &str) -> std::collections::HashMap<u32, bool> {
    let mut map = std::collections::HashMap::new();

    // Matches common forms:
    //   @group(0) @binding(1) var<storage, read> ...
    //   @binding(3) @group(0) var<storage, read_write> ...
    for line in wgsl.lines() {
        if !line.contains("var<storage,") {
            continue;
        }

        let read_only = if line.contains("var<storage, read_write>") {
            false
        } else if line.contains("var<storage, read>") {
            true
        } else {
            continue;
        };

        if let Some(binding_pos) = line.find("@binding(") {
            let start = binding_pos + "@binding(".len();
            if let Some(end_rel) = line[start..].find(')') {
                if let Ok(binding) = line[start..start + end_rel].trim().parse::<u32>() {
                    map.insert(binding, read_only);
                }
            }
        }
    }

    map
}

pub struct ComputeShader {
    pub(crate) pipeline:    ComputePipeline,
    pub(crate) bind_group:  BindGroup,
    uniform_buffer:         Option<Buffer>,
}

impl ComputeShader {
    pub fn new(
        ctx:          &GpuContext,
        wgsl:         &str,
        buffers:      &[&Buffer],
        uniform_size: u64,
    ) -> Self {
        let shader = ctx.device.create_shader_module(ShaderModuleDescriptor {
            label:  None,
            source: ShaderSource::Wgsl(wgsl.into()),
        });

        let has_uniform          = uniform_size > 0;
        let buffer_start_binding = if has_uniform { 1 } else { 0 };
        let storage_read_only    = parse_storage_read_only_map(wgsl);

        let mut layout_entries = Vec::new();

        if has_uniform {
            layout_entries.push(BindGroupLayoutEntry {
                binding:    0,
                visibility: ShaderStages::COMPUTE,
                ty: BindingType::Buffer {
                    ty:                  BufferBindingType::Uniform,
                    has_dynamic_offset:  false,
                    min_binding_size:    None,
                },
                count: None,
            });
        }

        layout_entries.extend(buffers.iter().enumerate().map(|(i, _)| {
            let binding = (buffer_start_binding + i) as u32;
            BindGroupLayoutEntry {
                binding,
                visibility: ShaderStages::COMPUTE,
                ty: BindingType::Buffer {
                    ty:                 BufferBindingType::Storage {
                        read_only: *storage_read_only.get(&binding).unwrap_or(&false),
                    },
                    has_dynamic_offset: false,
                    min_binding_size:   None,
                },
                count: None,
            }
        }));

        let bind_group_layout = ctx.device.create_bind_group_layout(
            &BindGroupLayoutDescriptor { label: None, entries: &layout_entries }
        );

        let pipeline_layout = ctx.device.create_pipeline_layout(&PipelineLayoutDescriptor {
            label:                None,
            bind_group_layouts:   &[&bind_group_layout],
            push_constant_ranges: &[],
        });

        let pipeline = ctx.device.create_compute_pipeline(&ComputePipelineDescriptor {
            label: None,
            layout: Some(&pipeline_layout),
            module: &shader,
            entry_point: "main",
            compilation_options: Default::default(),
        });

        let uniform_buffer = if uniform_size > 0 {
            Some(ctx.device.create_buffer(&BufferDescriptor {
                label:              Some("Uniform Buffer"),
                size:               uniform_size,
                usage:              BufferUsages::UNIFORM | BufferUsages::COPY_DST,
                mapped_at_creation: false,
            }))
        } else {
            None
        };

        let mut bg_entries = Vec::new();

        if let Some(ref ub) = uniform_buffer {
            bg_entries.push(BindGroupEntry {
                binding:  0,
                resource: ub.as_entire_binding(),
            });
        }

        bg_entries.extend(buffers.iter().enumerate().map(|(i, b)| BindGroupEntry {
            binding:  (buffer_start_binding + i) as u32,
            resource: b.as_entire_binding(),
        }));

        let bind_group = ctx.device.create_bind_group(&BindGroupDescriptor {
            label:   None,
            layout:  &bind_group_layout,
            entries: &bg_entries,
        });

        Self { pipeline, bind_group, uniform_buffer }
    }

    /// Write uniform data via the queue. Safe to call at any time before the
    /// encoder that dispatches this shader is submitted — wgpu stages writes
    /// and they are guaranteed visible to the next submission.
    pub fn set_uniform(&self, ctx: &GpuContext, data: &[u8]) {
        if let Some(ref buffer) = self.uniform_buffer {
            ctx.queue.write_buffer(buffer, 0, data);
        }
    }
}
