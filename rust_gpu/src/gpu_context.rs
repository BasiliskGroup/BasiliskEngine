use wgpu::*;

pub struct GpuContext {
    pub device: Device,
    pub queue:  Queue,
}

impl GpuContext {
    pub fn new() -> Self {
        let instance = Instance::new(InstanceDescriptor {
            backends: Backends::all(),
            flags: InstanceFlags::empty(),
            gles_minor_version: Gles3MinorVersion::Automatic,
            dx12_shader_compiler: Default::default(),
        });

        let adapter = pollster::block_on(instance.request_adapter(&RequestAdapterOptions {
            power_preference:       PowerPreference::HighPerformance,
            compatible_surface:     None,
            force_fallback_adapter: false,
        }))
        .expect("No suitable GPU adapter found");

        let (device, queue) = pollster::block_on(adapter.request_device(
            &DeviceDescriptor {
                label:    None,
                required_features: Features::empty(),
                required_limits: Limits::default(),
            },
            None,
        ))
        .expect("Device creation failed");

        Self { device, queue }
    }

    pub fn submit(&self, cmd: CommandBuffer) {
        self.queue.submit(Some(cmd));
    }

    /// Non-blocking poll — process any completed work without waiting.
    pub fn poll(&self) {
        self.device.poll(Maintain::Poll);
    }

    /// Blocking poll — stall CPU until all submitted GPU work is complete.
    pub fn poll_wait(&self) {
        self.device.poll(Maintain::Wait);
    }
}
