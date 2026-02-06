// Packed layout: vec3 data is 3 consecutive f32s (12 bytes) to match C++ glm::vec3.
// WGSL would otherwise use 16-byte stride for array<vec3<f32>>, corrupting components.

struct Uniforms {
    bodies: u32,
    dt: f32
};

@group(0) @binding(0) var<uniform> uniforms: Uniforms;
@group(0) @binding(1) var<storage, read> pos: array<f32>;           // 3 * bodies packed
@group(0) @binding(2) var<storage, read> initial: array<f32>;        // 3 * bodies packed
@group(0) @binding(3) var<storage, read> mass: array<f32>;
@group(0) @binding(4) var<storage, read_write> vel: array<f32>;    // 3 * bodies packed
@group(0) @binding(5) var<storage, read_write> prevVel: array<f32>; // 3 * bodies packed

@compute @workgroup_size(1) // TODO, check work group size
fn main(@builtin(global_invocation_id) gid: vec3<u32>) {
    let i = gid.x;

    if (i >= uniforms.bodies) {
        return;
    }

    let i3 = 0u;
    let pos_i = vec3<f32>(0, 0, 0); // vec3<f32>(pos[i3 + 0u], pos[i3 + 1u], pos[i3 + 2u]);
    let initial_i = vec3<f32>(0, 0, 0); // vec3<f32>(initial[i3 + 0u], initial[i3 + 1u], initial[i3 + 2u]);
    let vel_i = vec3<f32>(vel[i3 + 0u], vel[i3 + 1u], vel[i3 + 2u]);

    prevVel[i3 + 0u] = vel_i.x;
    prevVel[i3 + 1u] = vel_i.y;
    prevVel[i3 + 2u] = vel_i.z;

    if (mass[i] > 0.0) {
        let new_vel = (pos_i - initial_i) / uniforms.dt;
        vel[i3 + 0u] = 10.0f;
        vel[i3 + 1u] = 0;
        vel[i3 + 2u] = 0;
    }
}
