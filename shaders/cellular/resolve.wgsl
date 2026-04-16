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
@group(0) @binding(1) var<storage, read>       cells:        array<u32>;
@group(0) @binding(2) var<storage, read>       intent:       array<i32>;
@group(0) @binding(3) var<storage, read_write> claim:        array<i32>;
@group(0) @binding(4) var<storage, read>       chunk_intent: array<u32>;

fn idx(x: i32, y: i32) -> i32 {
    return y * i32(u.width) + x;
}

fn valid(x: i32, y: i32) -> bool {
    return x >= 0 && y >= 0 &&
           x < i32(u.width) &&
           y < i32(u.height);
}

fn material(c: u32) -> u32 {
    return (c >> 24u) & 0x0Fu;
}

fn is_empty(c: u32) -> bool {
    return material(c) == 0u;
}

fn chunk_idx(cx: i32, cy: i32) -> i32 {
    return cy * i32(u.chunks_wide) + cx;
}

fn claims(me: i32, sx: i32, sy: i32) -> bool {
    if (!valid(sx, sy)) { return false; }
    let dest_cell = cells[me];
    let wants_move_here = intent[idx(sx, sy)] == me;
    if (is_empty(dest_cell)) { return wants_move_here; }
    let wants_move_there = intent[me] == idx(sx, sy);
    return wants_move_here && wants_move_there;
}

@compute @workgroup_size(16, 16)
fn main(
    @builtin(global_invocation_id) gid:  vec3<u32>,
    @builtin(workgroup_id)         wgid: vec3<u32>
) {
    let my_chunk = chunk_idx(i32(wgid.x), i32(wgid.y));

    let x = i32(gid.x);
    let y = i32(gid.y);

    if (!valid(x, y)) { return; }

    let me = idx(x, y);
    claim[me] = -1;

    if (chunk_intent[my_chunk] == 0u) { return; }

    let dx_first  = select(1, -1, u.is_left_frame == 1u);
    let dx_second = -dx_first;

    // diagonals run first to encourage forming slopes (higher priority)
    if (claims(me, x + dx_first,  y + 1)) { claim[me] = idx(x + dx_first,  y + 1); return; }
    if (claims(me, x + dx_second, y + 1)) { claim[me] = idx(x + dx_second, y + 1); return; }
    if (claims(me, x + dx_first,  y - 1)) { claim[me] = idx(x + dx_first,  y - 1); return; }
    if (claims(me, x + dx_second, y - 1)) { claim[me] = idx(x + dx_second, y - 1); return; }

    // straight vertical (second priority)
    if (claims(me, x, y + 1)) { claim[me] = idx(x, y + 1); return; }
    if (claims(me, x, y - 1)) { claim[me] = idx(x, y - 1); return; }

    // horizontal (lowest priority — fluid only)
    if (claims(me, x + dx_first,  y)) { claim[me] = idx(x + dx_first,  y); return; }
    if (claims(me, x + dx_second, y)) { claim[me] = idx(x + dx_second, y); return; }
}