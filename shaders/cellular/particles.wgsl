struct Uniforms {
    dt: f32,
    num_particles: u32,
    gravity: f32,
    cell_width: f32,
    grid_width: u32,
    grid_height: u32,
    chunk_size: u32,
    chunks_wide: u32,
};

struct Particle {
    pos: vec2<f32>,
    vel: vec2<f32>,
    color: u32,
    _pad: u32,
};

@group(0) @binding(0) var<uniform> uniforms: Uniforms;
@group(0) @binding(1) var<storage, read_write> particles: array<Particle>;
@group(0) @binding(2) var<storage, read_write> cells: array<u32>;
@group(0) @binding(3) var<storage, read_write> chunk_active_out: array<u32>;
@group(0) @binding(4) var<storage, read_write> free_stack: array<u32>;
@group(0) @binding(5) var<storage, read_write> free_count: array<atomic<u32>>;

fn cell_coords_from_pos(pos: vec2<f32>) -> vec2<i32> {
    return vec2<i32>(
        i32(floor(pos.x / uniforms.cell_width)),
        i32(floor(pos.y / uniforms.cell_width))
    );
}

fn cell_idx(x: i32, y: i32) -> i32 {
    return y * i32(uniforms.grid_width) + x;
}

fn in_bounds(x: i32, y: i32) -> bool {
    return x >= 0 && y >= 0 &&
           x < i32(uniforms.grid_width) &&
           y < i32(uniforms.grid_height);
}

fn material(c: u32) -> u32 {
    return (c >> 24u) & 0x1Fu;
}

fn is_empty(c: u32) -> bool {
    return material(c) == 0u;
}

fn particle_color_to_deposited_cell(packed: u32) -> u32 {
    // `packed` already matches CPU `packCell` / grid cells:
    //   bits 0-7 b, 8-15 g, 16-23 r, 24-28 mat, 29 fire, 30-31 momentum.
    // Older code assumed byte order r,g,b,mat (swapped R/B) and dropped fire — fix: pass through.
    return packed & ~(3u << 30u);
}

fn chunk_idx_for_cell(x: i32, y: i32) -> i32 {
    let cx = x / i32(uniforms.chunk_size);
    let cy = y / i32(uniforms.chunk_size);
    return cy * i32(uniforms.chunks_wide) + cx;
}

fn try_deposit_particle_cell(p: ptr<function, Particle>, x: i32, y: i32) -> bool {
    if (!in_bounds(x, y)) {
        return false;
    }

    let cur_idx = cell_idx(x, y);
    if (!is_empty(cells[cur_idx])) {
        return false;
    }

    // Prototype support check: bottom boundary counts as support,
    // otherwise require a filled cell directly below.
    var supported = (y == 0);
    if (!supported) {
        let below_idx = cell_idx(x, y - 1);
        supported = !is_empty(cells[below_idx]);
    }
    if (!supported) {
        return false;
    }

    cells[cur_idx] = particle_color_to_deposited_cell((*p).color);
    let ci = chunk_idx_for_cell(x, y);
    if (ci >= 0) {
        chunk_active_out[ci] = 1u;
    }
    // Deactivate and push index to free stack.
    (*p)._pad = 0u;
    (*p).pos = vec2<f32>(-1000.0, -1000.0);
    (*p).vel = vec2<f32>(0.0, 0.0);
    return true;
}

// TODO allow particles to collide in any direction

@compute @workgroup_size(256)
fn main(@builtin(global_invocation_id) global_id: vec3<u32>) {
    let index = global_id.x;
    if (index >= uniforms.num_particles) {
        return;
    }

    var p = particles[index];
    if ((p._pad & 1u) == 0u) {
        return;
    }
    let before_cell = cell_coords_from_pos(p.pos);

    // if we are currently in a cell, forcefully terminate particle
    if (in_bounds(before_cell.x, before_cell.y) &&
        material(cells[cell_idx(before_cell.x, before_cell.y)]) != 0u) {
        p.pos = vec2<f32>(-1000.0, -1000.0);
        p.vel = vec2<f32>(0.0, 0.0);
        p._pad = 0u;
    }

    p.vel.y -= uniforms.gravity * uniforms.dt;
    p.pos += p.vel * uniforms.dt;

    // Simple robust prototype: attempt deposit at post-integrated cell,
    // then also try the previous cell to reduce missed landings.
    let after_cell = cell_coords_from_pos(p.pos);
    let landed_after = try_deposit_particle_cell(&p, after_cell.x, after_cell.y);
    if (!landed_after) {
        let _landed_before = try_deposit_particle_cell(&p, before_cell.x, before_cell.y);
    }

    // If particle became inactive this step, publish index to free stack.
    if ((p._pad & 1u) == 0u) {
        let slot = atomicAdd(&free_count[0], 1u);
        if (slot < uniforms.num_particles) {
            free_stack[slot] = index;
        } else {
            _ = atomicSub(&free_count[0], 1u);
        }
    }

    particles[index] = p;
}