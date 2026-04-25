struct Uniforms {
    width: u32,
    height: u32,
    is_left_frame: u32,
    chunk_size: u32,
    chunks_wide: u32,
    random_seed: u32,
    _pad0: u32,
    _pad1: u32,
};

struct Particle {
    pos: vec2<f32>,
    vel: vec2<f32>,
    color: u32,
    _pad: f32,
};

// cell layout:
// | momentum (2bit) | onFire (1bit) | static (1bit) | material (4bit) | red (8bit) | green (8bit) | blue (8bit) |
// material id (bits 27-24), 0-15:
// static (bit 28): user-controlled property bit
// onFire (bit 29): 1 = burning (reserved for sim / rendering)
// momentum (bits 31-30):
// 0: none, 1: left, 2: right

@group(0) @binding(0) var<uniform> u: Uniforms;
@group(0) @binding(1) var<storage, read_write> cells:             array<u32>;
@group(0) @binding(2) var<storage, read_write> intent:            array<i32>;
@group(0) @binding(3) var<storage, read>       active_chunk_list: array<u32>;
@group(0) @binding(4) var<storage, read_write> chunk_intent:      array<u32>;

// @group(0) @binding(5) var<storage, read_write> particles: array<Particle>;
// @group(0) @binding(6) var<storage, read_write> free_stack: array<u32>;
// @group(0) @binding(7) var<storage, read_write> free_count: array<atomic<u32>>;

const FIRE_BIT: u32 = 29u;
const STATIC_BIT: u32 = 28u;

const EMPTY:    u32 = 0u;
const SOLID:    u32 = 1u;
const WATER:    u32 = 2u;
const GAS:      u32 = 3u;
const WOOD:     u32 = 4u;
const OBSIDIAN: u32 = 5u;
const FUSE:     u32 = 6u;
const MUD:      u32 = 7u;
const CLAY:     u32 = 8u;
const GLASS:    u32 = 9u;
const METAL:    u32 = 10u;
const PLASTIC:  u32 = 11u;
const SNOW:     u32 = 12u;
const VAPOR:    u32 = 13u;

// ------------------------------------------------------------
// Locality function
// ------------------------------------------------------------

fn idx(x: i32, y: i32) -> i32 { return y * i32(u.width) + x; }
fn chunk_idx(cx: i32, cy: i32) -> i32 { return cy * i32(u.chunks_wide) + cx; }

fn valid(x: i32, y: i32) -> bool {
    return x >= 0 && y >= 0 &&
           x < i32(u.width) &&
           y < i32(u.height);
}

// ------------------------------------------------------------
// Metadata
// ------------------------------------------------------------

fn material(c: u32) -> u32 { 
    return (c >> 24u) & 0x0Fu; 
}

fn is_static(c: u32) -> u32 { 
    return (c >> STATIC_BIT) & 1u ; 
}

fn can_move(c: u32) -> bool { 
    return is_static(c) == 0u && material(c) != EMPTY; 
}

fn on_fire(c: u32) -> u32 { 
    return (c >> FIRE_BIT) & 1u; 
}

fn momentum(c: u32) -> u32 { 
    return (c >> 30u) & 0x3u; 
}

fn is_empty(c: u32) -> bool { 
    return material(c) == EMPTY; 
}

fn is_floater(c: u32) -> bool { 
    return material(c) == GAS || material(c) == VAPOR; 
}

fn is_denser(a: u32, b: u32) -> bool { 
    return density(a) > density(b); 
}

fn is_less_dense(a: u32, b: u32) -> bool { 
    return density(a) < density(b); 
}

fn set_fire(c: u32, fire: u32) -> u32 {
    return (c & ~(1u << FIRE_BIT)) | ((fire & 1u) << FIRE_BIT);
}

fn set_static(c: u32, me_static: u32) -> u32 {
    return (c & ~(1u << STATIC_BIT)) | ((me_static & 1u) << STATIC_BIT);
}

fn set_material(c: u32, mat: u32) -> u32 {
    return (c & ~(0x1Fu << 24u)) | ((mat & 0x1Fu) << 24u);
}

fn set_color(c: u32, r: u32, g: u32, b: u32) -> u32 {
    return (c & ~0xFFFFFFu) | ((r & 0xFFu) << 16u) | ((g & 0xFFu) << 8u) | (b & 0xFFu);
}

// ------------------------------------------------------------
// Material-Specific Properties
// ------------------------------------------------------------
// Naga/wgpu: const arrays may only be indexed by constant expressions — use switch for runtime mat id.

fn density(c: u32) -> u32 {
    let mat = material(c);
    let me_static = is_static(c);
    let fluid = is_fluid(c);
    var density: u32;
    switch mat {
        case EMPTY: { density = 0u; }
        case SOLID: { density = 10u; }
        case WATER: { density = 12u; }
        case GAS: { density = 1u; }
        case WOOD: { density = 10u; }
        case OBSIDIAN: { density = 10u; }
        case FUSE: { density = 10u; }
        case MUD: { density = 6u; }
        case CLAY: { density = 10u; }
        case GLASS: { density = 10u; }
        case METAL: { density = 10u; }
        case PLASTIC: { density = 10u; }
        case SNOW: { density = 10u; }
        case VAPOR: { density = 2u; }

        default: { density = 10u; }
    }
    return density + 10u * me_static + 5u * (1u - u32(fluid));
}

fn flamability(c: u32) -> f32 {
    let mat = material(c);
    let me_static = is_static(c);
    switch mat {
        case EMPTY: { return 0.0; }
        case SOLID: { return 0.001; }
        case WATER: { return 0.0; }
        case GAS: { return 0.5; }
        case WOOD: { return 0.01; }
        case OBSIDIAN: { return 0.0; }
        case FUSE: { return 0.1; }
        case MUD: { return 0.01; }
        case CLAY: { return 0.0; }
        case GLASS: { return 0.0; } // TODO this melt 
        case METAL: { return 0.01; }
        case PLASTIC: { return 0.05; }
        case SNOW: { return 1.0; }
        case VAPOR: { return 0.0; }

        default: { return 0.0; }
    }
}

fn extinguishability(c: u32) -> f32 {
    let mat = material(c);
    let me_static = is_static(c);
    switch mat {
        case EMPTY: { return 1.0; }
        case SOLID: { return 0.01; }
        case WATER: { return 0.0; }
        case GAS: { return 0.3; }
        case WOOD: { return 0.01; }
        case OBSIDIAN: { return 0.0; }
        case FUSE: { return 0.1; }  
        case MUD: { return 0.0; }
        case CLAY: { return 0.0; }
        case GLASS: { return 0.0; } // TODO make this melt
        case METAL: { return 0.02; }
        case PLASTIC: { return 0.2; }
        case SNOW: { return 0.0; } // melt into water
        case VAPOR: { return 0.0; }

        default: { return 0.01; }
    }
}

fn is_fluid(c: u32) -> bool {
    let mat = material(c);
    switch mat {
        case EMPTY: { return false; }
        case SOLID: { return false; }
        case WATER: { return true; }
        case GAS: { return true; }
        case WOOD: { return false; }
        case OBSIDIAN: { return false; }
        case FUSE: { return false; }
        case MUD: { return false; }
        case CLAY: { return false; }
        case GLASS: { return false; }
        case METAL: { return on_fire(c) == 1u; }
        case PLASTIC: { return on_fire(c) == 1u; }
        case SNOW: { return false; }
        case VAPOR: { return true; }

        default: { return false; }
    }
}

// ------------------------------------------------------------
// Destination computation
// ------------------------------------------------------------

fn dest_if_empty(nx: i32, ny: i32) -> i32 {
    if (!valid(nx, ny)) { return -1; }
    let v = cells[idx(nx, ny)];
    if (is_empty(v)) {
        return idx(nx, ny);
    }
    return -1;
}

fn dest_if_denser(mover: u32, nx: i32, ny: i32) -> i32 {
    if (!valid(nx, ny)) { return -1; }
    let v = cells[idx(nx, ny)];
    if (is_denser(mover, v)) {
        return idx(nx, ny);
    }
    return -1;
}

// only works if destination is not empty
fn dest_if_less_dense(mover: u32, nx: i32, ny: i32) -> i32 {
    if (!valid(nx, ny)) { return -1; }
    let v = cells[idx(nx, ny)];
    if (is_empty(v)) { return -1; }
    if (is_less_dense(mover, v)) {
        return idx(nx, ny);
    }
    return -1;
}

// ------------------------------------------------------------
// randomization
// ------------------------------------------------------------

fn hash_u32(x: u32) -> u32 {
    var v = x;
    v = v ^ (v >> 16u);
    v *= 0x7feb352du;
    v = v ^ (v >> 15u);
    v *= 0x846ca68bu;
    v = v ^ (v >> 16u);
    return v;
}

fn rand01(a: u32, b: u32) -> f32 {
    let h = hash_u32(a ^ (hash_u32(b) << 1u));
    return f32(h & 0xFFFFFFu) / f32(0x1000000u);
}

// ------------------------------------------------------------
// Main intent computation
// ------------------------------------------------------------

@compute @workgroup_size(16, 16)
fn main(
    @builtin(local_invocation_id)  lid:  vec3<u32>,
    @builtin(workgroup_id)         wgid: vec3<u32>
) {
    let my_chunk = i32(active_chunk_list[wgid.x]);
    let cx = my_chunk % i32(u.chunks_wide);
    let cy = my_chunk / i32(u.chunks_wide);

    let x = cx * i32(u.chunk_size) + i32(lid.x);
    let y = cy * i32(u.chunk_size) + i32(lid.y);

    if (valid(x, y) == false) { return; }

    let src  = idx(x, y);
    let cell = cells[src];

    intent[src] = -1;

    let mat = material(cell);

    // fire
    let is_fire = on_fire(cell);
    if (is_fire == 1u) {

        // check if cell has alternate heated behavior
        if (mat == MUD) {
            cells[src] = set_material(set_fire(cell, 0u), CLAY);       
            return;
        }

        if (mat == SNOW) {
            cells[src] = set_color(
                set_material(set_fire(cell, 0u), WATER),
                70u,
                130u,
                220u,
            );
            return;
        }

        if ((mat == METAL || mat == PLASTIC) && is_static(cell) == 1u) {
            cells[src] = set_static(cell, 0u);       
        }

        // chance to die
        let r_fire = rand01(u32(src), u.random_seed);
        let extinguish = extinguishability(cell);
        if (r_fire < extinguish) {

            if (mat == METAL || mat == PLASTIC) {
                cells[src] = set_fire(cell, 0u);       
                return;
            }

            cells[src] = 0u;
            return;
        }

        // set surroundings on fire
        for (var fdy: i32 = -1; fdy <= 1; fdy++) {
            for (var fdx: i32 = -1; fdx <= 1; fdx++) {
                if (fdx == 0 && fdy == 0) {
                    continue;
                }

                // bounds check
                let nx = x + fdx;
                let ny = y + fdy;
                if (!valid(nx, ny)) {
                    continue;
                }

                // empty check
                let n = idx(nx, ny);
                let nc = cells[n];
                if (is_empty(nc)) {
                    continue;
                }

                // spread chance
                let r_spread = rand01(u32(src) ^ (u32(n) * 0x9E3779B9u), u.random_seed ^ 0xA341316Cu);
                let flamability = flamability(nc);
                if (r_spread < flamability) {
                    cells[n] = nc | (1u << FIRE_BIT);
                }
            }
        }
    }

    if (can_move(cell) == false) { return; }

    // Gravity step in y: fall for sand/water (dy < 0), rise for gas (dy > 0).
    let dy = select(-1, 1, is_floater(cell));

    // dx_first: momentum takes priority, then is_left_frame tiebreaker
    let mom = momentum(cell);
    var dx_first: i32;
    if (mom == 1u) { dx_first = -1;
    } else if (mom == 2u) { dx_first = 1;
    } else { dx_first = select(1, -1, u.is_left_frame == 1u); }
    let dx_second = -dx_first;

    // init dest to -1 (no destination)
    var dest: i32 = -1;

    // Down — empty
    if (dest < 0) { dest = dest_if_empty(x, y + dy); }

    // Down — swap (displace)
    if (dest < 0) { dest = dest_if_denser(cell, x, y + dy); }

    // Horizontal — empty (fluids only; same dx order as before)
    if (dest < 0 && is_fluid(cell)) { dest = dest_if_empty(x + dx_first, y); }
    if (dest < 0 && is_fluid(cell)) { dest = dest_if_empty(x + dx_second, y); }

    // Down diagonal — empty (dx_first, then dx_second)
    if (dest < 0) { dest = dest_if_empty(x + dx_first, y + dy); }
    if (dest < 0) { dest = dest_if_empty(x + dx_second, y + dy); }

    // Down diagonal — swap (dx_first, then dx_second)
    if (dest < 0) { dest = dest_if_denser(cell, x + dx_first, y + dy); }
    if (dest < 0) { dest = dest_if_denser(cell, x + dx_second, y + dy); }

    // +Y: less-dense swap (floaters need y + 1; y - dy is wrong when dy == 1).
    if (dest < 0) { dest = dest_if_less_dense(cell, x, y + 1); }
    if (dest < 0) { dest = dest_if_less_dense(cell, x + dx_first, y + 1); }
    if (dest < 0) { dest = dest_if_less_dense(cell, x + dx_second, y + 1); }

    // if no destination, return
    if (dest < 0) { return; }

    intent[src] = dest;

    // mark my chunk as intent
    chunk_intent[my_chunk] = 1u;

    let dest_x     = dest % i32(u.width);
    let dest_y     = dest / i32(u.width);
    let dest_cx    = dest_x / i32(u.chunk_size);
    let dest_cy    = dest_y / i32(u.chunk_size);
    let dest_chunk = chunk_idx(dest_cx, dest_cy);
    if (dest_chunk != my_chunk) {
        chunk_intent[dest_chunk] = 1u;
    }
}