use wgpu::*;
use std::env;

pub struct GpuContext {
    pub device: Device,
    pub queue:  Queue,
}

impl GpuContext {
    pub fn new() -> Self {
        // Avoid the GLES/EGL backend by default on Linux because it can fail at runtime
        // with EGL_BAD_ACCESS depending on driver/display stack (Wayland/X11/headless).
        // Vulkan is the most reliable backend for compute here.
        let backends = match env::var("BSK_WGPU_BACKENDS").ok().as_deref() {
            Some("vulkan") => Backends::VULKAN,
            Some("gl") | Some("gles") => Backends::GL,
            Some("all") => Backends::all(),
            Some(_) | None => {
                // Linux: avoid GLES/EGL stacks that can fail with EGL_BAD_ACCESS.
                // Windows: default wgpu backend order can pick D3D12 while GLFW uses OpenGL;
                // some drivers deadlock or hang indefinitely in compute pipeline creation
                // when both APIs are active in one process — Vulkan is reliable here.
                #[cfg(any(target_os = "linux", target_os = "windows"))]
                {
                    Backends::VULKAN
                }
                #[cfg(not(any(target_os = "linux", target_os = "windows")))]
                {
                    Backends::all()
                }
            }
        };

        let instance = Instance::new(InstanceDescriptor {
            backends,
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
