use wgpu::*;

pub struct GpuContext {
    pub device: Device,
    pub queue: Queue,
}

impl GpuContext {
    // constructor
    pub fn new () -> Self {
        // Select backends based on platform. On Linux, we primarily use Vulkan.
        // Note: The C++ wrapper (gpuWrapper.hpp) will skip GPU init entirely
        // when ASan is enabled on Linux to avoid heap-use-after-free in libvulkan.
        #[cfg(target_os = "macos")]
        let backends = Backends::METAL | Backends::VULKAN;
        
        #[cfg(target_os = "linux")]
        let backends = Backends::VULKAN;
        
        #[cfg(target_os = "windows")]
        let backends = Backends::DX12 | Backends::VULKAN;
        
        #[cfg(not(any(target_os = "macos", target_os = "linux", target_os = "windows")))]
        let backends = Backends::METAL | Backends::VULKAN | Backends::DX12;
        
        let instance = Instance::new(InstanceDescriptor {
            backends,
            dx12_shader_compiler: Default::default()
        });

        let adapter = pollster::block_on(instance.request_adapter(
            &RequestAdapterOptions {
                power_preference: PowerPreference::HighPerformance,
                compatible_surface: None,
                force_fallback_adapter: false
            }
        ))
            .expect("No adapter");

        let (device, queue) = pollster::block_on(adapter.request_device(
            &DeviceDescriptor {
                label: None,
                features: Features::empty(),
                limits: Limits::default()
            },
            None
        ))
            .expect("Device creaton failed");

        Self { device, queue }
    }

    // submit command buffer to GPU queue
    pub fn submit(&self, cmd: CommandBuffer) {
        self.queue.submit(Some(cmd));
    }

    // poll the device to process submitted work
    pub fn poll(&self, wait: bool) {
        match wait {
            true => self.device.poll(Maintain::Wait),
            false => self.device.poll(Maintain::Poll),
        };
    }
}