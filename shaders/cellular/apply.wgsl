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

@group(0) @binding(0) var<uniform> u: Uniforms;
@group(0) @binding(1) var<storage, read>       cells:            array<u32>;
@group(0) @binding(2) var<storage, read>       intent:           array<i32>;
@group(0) @binding(3) var<storage, read>       claim:            array<i32>;
@group(0) @binding(4) var<storage, read_write> out_cells:        array<u32>;
@group(0) @binding(5) var<storage, read_write> chunk_active_out: array<u32>;
@group(0) @binding(6) var<storage, read>       chunk_intent:     array<u32>;
@group(0) @binding(7) var<storage, read>       chunk_active:     array<u32>;

fn empty() -> u32 {
    return 0x00000000u;
}

fn chunk_idx(cx: i32, cy: i32) -> i32 {
    return cy * i32(u.chunks_wide) + cx;
}

fn set_momentum(c: u32, mom: u32) -> u32 {
    return (c & 0x3FFFFFFFu) | ((mom & 0x3u) << 30u);
}

const FIRE_BIT: u32 = 29u;
fn on_fire(c: u32) -> u32 { return (c >> FIRE_BIT) & 1u; }

@compute @workgroup_size(16, 16)
fn main(
    @builtin(global_invocation_id) gid:  vec3<u32>,
    @builtin(workgroup_id)         wgid: vec3<u32>
) {
    let my_chunk = chunk_idx(i32(wgid.x), i32(wgid.y));
    let id       = i32(gid.x) + i32(gid.y) * i32(u.width);
    let max_id   = i32(u.width * u.height);

    if (id >= max_id) { return; }

    // if we are on fire, set chunk to active
    if (on_fire(cells[id]) == 1u) {
        chunk_active_out[my_chunk] = 1u;
    }

    if (chunk_intent[my_chunk] == 0u) {
        out_cells[id] = cells[id];
        return;
    }

    if (chunk_active[my_chunk] != 0u && intent[id] >= 0) {
        if (claim[intent[id]] == id) {
            let dest = intent[id];
            let src_x  = id   % i32(u.width);
            let dest_x = dest % i32(u.width);
            let dx = dest_x - src_x;
            var mom: u32 = 0u;
            if (dx < 0) { mom = 1u; }
            if (dx > 0) { mom = 2u; }
            // Move self to destination only — victim writes itself back separately
            out_cells[dest] = set_momentum(cells[id], mom);
            chunk_active_out[my_chunk] = 1u;
        } else {
            out_cells[id] = set_momentum(cells[id], 0u);
        }
    } else if (claim[id] >= 0) {
        // Someone is moving into our slot. Write ourselves to their old position.
        let winner_src = claim[id];
        out_cells[winner_src] = set_momentum(cells[id], 0u);
        chunk_active_out[my_chunk] = 1u;
    } else {
        out_cells[id] = set_momentum(cells[id], 0u);
    }
}