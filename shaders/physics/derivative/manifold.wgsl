// structs
struct Uniforms {
    forces: u32,
    alpha: f32,
    beta: f32
};

struct Contact {
    rA: vec2<f32>,
    rB: vec2<f32>,
    normal: vec2<f32>,
    C0: vec2<f32>,
    stick: u32,
    _padding: u32
}

struct ManifoldData {
    contacts: array<Contact, 2>,
    numContacts: u32,
    friction: f32
}

struct ParameterStruct {
    C: f32,
    fmin: f32,
    fmax: f32,
    stiffness: f32,
    fracture: f32,
    penalty: f32,
    lambda: f32
};

struct DerivativeStruct {
    J: vec3<f32>,
    H: mat3x3<f32>
}

struct Positional {
    pos: array<vec3<f32>, 2>,
    initial: array<vec3<f32>, 2>
}

// bindings
@group(0) @binding(0) var<uniform> uniforms: Uniforms;
@group(0) @binding(1) var<storage, read> forceType: array<u32>; 
@group(0) @binding(2) var<storage, read_write> params: array<ParameterStruct>; // 4 per force
@group(0) @binding(3) var<storage, read> forceRows: array<u32>;
@group(0) @binding(4) var<storage, read> positional: array<Positional>;
@group(0) @binding(5) var<storage, read_write> manifolds: array<ManifoldData>;

// function
fn computeConstraint(m: u32) {
    // assume index is in bounds
    for (var i: u32 = 0u; i < manifolds[m].numContacts; i = i + 1u) {
        
    }
}