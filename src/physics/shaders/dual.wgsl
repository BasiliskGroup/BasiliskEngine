// ------------------------------------------------------------
// Struct Definitions
// ------------------------------------------------------------
struct Uniforms {
    forces: u32,
    beta: f32
};

struct ParameterStruct {
    C: f32,
    fmin: f32,
    fmax: f32,
    stiffness: f32,
    fracture: f32,
    penalty: f32,
    lambda: f32
};

// ------------------------------------------------------------
// Bindings
// ------------------------------------------------------------

@group(0) @binding(0) var<uniform> uniforms: Uniforms;
@group(0) @binding(1) var<storage, read> forceType: array<u32>; 
@group(0) @binding(2) var<storage, read_write> forceParameters: array<ParameterStruct>; // 4 per force
@group(0) @binding(3) var<storage, read> forceRows: array<u32>;

// ------------------------------------------------------------
// Functions
// ------------------------------------------------------------

fn isInf(x: f32) -> bool {
    return (x == 1.0 / 0.0) | (x == -1.0 / 0.0);
}

fn disableForce(i: u32) {
    for (var j: u32 = i; j < i + 4u; j = j + 1u) {
        let params& = forceParameters[j];
        params.stiffness = 0.0;
        params.penalty = 0.0;
        params.lambda = 0.0;
    }
}

@compute @workgroup_size(128)
fn main(@builtin(global_invocation_id) gid: vec3<u32>) {
    let i = gid.x;

    if (i >= uniforms.forces) {
        return;
    }

    // update force parameters
    for(var j: u32 = i; j < i + 4u; j = j + 1u) {
        let params& = forceParameters[j];

        // TODO force-specific compute constraint
        // this uses a switch statement in C++ to call static functions, one for each of 4 force types
        
        // use lambda if it is a hard constraint
        let lambda=  select(params.lambda, 0.0, isInf(params.stiffness));

        // update lambda (Eq 11)
        params.lambda = clamp(params.penalty * params.C + lambda, params.fmin, params.fmax);

        // disable the force if it has exceeded its fracture threshold
        if (abs(params.lambda) >= params.fracture) {
            disableForce(i);
            return; // if we disable, we can stop processing this force
        }

        // update the penalty parameter and clamp to material stiffness if we are within the force bounds (Eq. 16)
        if (params.lambda > params.fmin && params.lambda < params.fmax) {
            params.penalty = clamp(params.penalty + uniforms.beta * abs(params.C), 0.0, params.stiffness);
        }
    }
}