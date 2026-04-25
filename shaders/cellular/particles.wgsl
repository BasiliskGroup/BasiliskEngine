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
    _pad: f32,
};

@group(0) @binding(0) var<uniform> uniforms: Uniforms;
@group(0) @binding(1) var<storage, read_write> particles: array<Particle>;
@group(0) @binding(2) var<storage, read_write> cells: array<u32>;
@group(0) @binding(3) var<storage, read_write> chunk_active_out: array<u32>;
@group(0) @binding(4) var<storage, read_write> free_stack: array<u32>;
@group(0) @binding(5) var<storage, read_write> free_count: array<atomic<u32>>;

fn cell_coords_from_pos(pos: vec2<f32>) -> vec2<i32> {
    // Particle positions are tracked in grid pixel coordinates on CPU and GPU.
    // Do not scale by cell_width here or collision probes drift from terrain cells.
    return vec2<i32>(
        i32(floor(pos.x)),
        i32(floor(pos.y))
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

    // Support check: bottom boundary counts as support,
    // otherwise require at least one filled cell among all 8 neighbors.
    var supported = (y == 0);
    if (!supported) {
        // Unrolled neighbor checks avoid dynamic indexing limitations on some backends.
        if (in_bounds(x - 1, y - 1) && !is_empty(cells[cell_idx(x - 1, y - 1)])) { supported = true; }
        if (!supported && in_bounds(x, y - 1) && !is_empty(cells[cell_idx(x, y - 1)])) { supported = true; }
        if (!supported && in_bounds(x + 1, y - 1) && !is_empty(cells[cell_idx(x + 1, y - 1)])) { supported = true; }
        if (!supported && in_bounds(x - 1, y) && !is_empty(cells[cell_idx(x - 1, y)])) { supported = true; }
        if (!supported && in_bounds(x + 1, y) && !is_empty(cells[cell_idx(x + 1, y)])) { supported = true; }
        if (!supported && in_bounds(x - 1, y + 1) && !is_empty(cells[cell_idx(x - 1, y + 1)])) { supported = true; }
        if (!supported && in_bounds(x, y + 1) && !is_empty(cells[cell_idx(x, y + 1)])) { supported = true; }
        if (!supported && in_bounds(x + 1, y + 1) && !is_empty(cells[cell_idx(x + 1, y + 1)])) { supported = true; }
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
    (*p)._pad = -1.0;
    (*p).pos = vec2<f32>(-1000.0, -1000.0);
    (*p).vel = vec2<f32>(0.0, 0.0);
    return true;
}


@compute @workgroup_size(256)
fn main(@builtin(global_invocation_id) global_id: vec3<u32>) {
    let index = global_id.x;
    if (index >= uniforms.num_particles) {
        return;
    }

    var p = particles[index];
    if (p._pad < 0.0) {
        return;
    }
    let was_forced_active = p._pad > 0.0;
    if (p._pad > 0.0) {
        p._pad = max(0.0, p._pad - uniforms.dt);
    }
    let before_cell = cell_coords_from_pos(p.pos);

    // Velocity/gravity are in world units per second; positions are in cell units.
    // Convert world displacement to cell displacement via cell_width.
    p.vel.y -= uniforms.gravity * uniforms.dt;
    p.pos += (p.vel * uniforms.dt) / uniforms.cell_width;

    let after_cell = cell_coords_from_pos(p.pos);
    let after_in_bounds = in_bounds(after_cell.x, after_cell.y);

    if (was_forced_active && after_in_bounds) {
        particles[index] = p;
        return;
    }

    if (!after_in_bounds) {
        let border_x = clamp(after_cell.x, 0, i32(uniforms.grid_width) - 1);
        let border_y = clamp(after_cell.y, 0, i32(uniforms.grid_height) - 1);
        let _landed_border = try_deposit_particle_cell(&p, border_x, border_y);
        if (p._pad >= 0.0) {
            p.pos = vec2<f32>(-1000.0, -1000.0);
            p.vel = vec2<f32>(0.0, 0.0);
            p._pad = -1.0;
        }
        if (p._pad < 0.0) {
            let slot = atomicAdd(&free_count[0], 1u);
            if (slot < uniforms.num_particles) {
                free_stack[slot] = index;
            } else {
                _ = atomicSub(&free_count[0], 1u);
            }
        }
        particles[index] = p;
        return;
    }

    // End-of-frame overlap check: if the new position is inside a filled cell,
    // deposit sand at the previous cell position instead of the new one.
    let after_in_sand = material(cells[cell_idx(after_cell.x, after_cell.y)]) != 0u;
    if (after_in_sand) {
        // Try to place sand where the particle was before moving.
        let _landed_before = try_deposit_particle_cell(&p, before_cell.x, before_cell.y);
        // If that also failed (e.g. before_cell is occupied too), just deactivate.
        if (p._pad >= 0.0) {
            p.pos = vec2<f32>(-1000.0, -1000.0);
            p.vel = vec2<f32>(0.0, 0.0);
            p._pad = -1.0;
        }
    } else {
        // Normal landing: attempt deposit at post-integrated cell,
        // then also try the previous cell to reduce missed landings.
        let landed_after = try_deposit_particle_cell(&p, after_cell.x, after_cell.y);
        if (!landed_after) {
            let _landed_before = try_deposit_particle_cell(&p, before_cell.x, before_cell.y);
        }
    }

    // If particle became inactive this step, publish index to free stack.
    if (p._pad < 0.0) {
        let slot = atomicAdd(&free_count[0], 1u);
        if (slot < uniforms.num_particles) {
            free_stack[slot] = index;
        } else {
            _ = atomicSub(&free_count[0], 1u);
        }
    }

    particles[index] = p;
}