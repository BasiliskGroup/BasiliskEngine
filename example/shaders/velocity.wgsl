struct Uniforms {
    bodies: u32,
    dt: f32
};

@group(0) @binding(0) var<uniform> uniforms: Uniforms;
@group(0) @binding(1) var<storage, read> pos: array<vec3<f32>>;
@group(0) @binding(2) var<storage, read> initial: array<vec3<f32>>;
@group(0) @binding(3) var<storage, read> mass: array<f32>;
@group(0) @binding(4) var<storage, read_write> vel: array<vec3<f32>>;
@group(0) @binding(5) var<storage, read_write> prevVel: array<vec3<f32>>;

@compute @workgroup_size(128)
fn main(@builtin(global_invocation_id) gid: vec3<u32>) {
    let i = gid.x;

    if (i >= uniforms.bodies) {
        return;
    }

    prevVel[i] = vel[i];

    if (mass[i] > 0.0) {
        vel[i] = (pos[i] - initial[i]) / uniforms.dt;
    }
}
