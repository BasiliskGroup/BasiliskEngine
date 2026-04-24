import math
import basilisk as bsk
import glm


WINDOW_WIDTH = 800
WINDOW_HEIGHT = 800
MAX_MATERIAL_ID = 15
STATIC_MAT_BIT = 1 << 4


def base_brush_color_for_material(mat_id: int) -> tuple[int, int, int]:
    palette = {
        0: (0, 0, 0),
        1: (170, 120, 90),
        2: (70, 130, 220),
        3: (160, 200, 160),
        4: (120, 80, 45),
        5: (35, 30, 45),
        6: (200, 110, 50),
        7: (125, 95, 70),
        8: (170, 130, 95),
        9: (185, 215, 230),
        10: (155, 160, 170),
        11: (70, 150, 170),
        12: (200, 200, 200),
        13: (170, 170, 170),
    }
    return palette.get(mat_id, (90, 90, 90))


def rainbow_brush_color(engine: bsk.Engine, mat_id: int, on_fire: bool) -> bsk.Color:
    t = engine.get_window().get_time() / 0.5
    r0, g0, b0 = base_brush_color_for_material(mat_id)

    def wave(base_channel: int, phase: float) -> int:
        return max(0, min(255, int(round(base_channel + 40.0 * math.sin(phase)))))

    return bsk.Color(wave(r0, t), wave(g0, t), wave(b0, t), mat_id, 1 if on_fire else 0)


def main() -> None:
    engine = bsk.Engine(WINDOW_WIDTH, WINDOW_HEIGHT, "Sand Simulation", False, False)
    scene = bsk.Scene2D(engine)
    scene.camera = bsk.Camera2D(engine, glm.vec2(0.0, 0.0), 30.0)
    scene.camera.speed = 30.0

    cell_buffer = scene.get_solver().get_cell_buffer()
    buffer_width = cell_buffer.get_width()
    buffer_height = cell_buffer.get_height()
    cell_buffer.initialize_compute()

    mat_id = 1
    particle_mode = False
    fire_mode = False
    static_mode = False

    sand_shader = bsk.Shader("shaders/sand.vert", "shaders/sand.frag")
    sand_frame = bsk.Frame(engine, sand_shader, buffer_width, buffer_height)
    camera_scale = scene.get_camera().get_view_scale()
    collider = bsk.Collider([(0.5, 0.5), (-0.5, 0.5), (-0.5, -0.5), (0.5, -0.5)]);

    while engine.is_running():
        engine.update()
        scene.update()

        mouse = engine.get_mouse()
        keyboard = engine.get_keyboard()
        camera = scene.get_camera()

        mouse_pos = glm.vec2(mouse.get_world_x(camera), mouse.get_world_y(camera))

        if keyboard.get_pressed(bsk.key.KeyCode.K_SPACE):
            particle_mode = not particle_mode
            print(f"Brush mode: {'particles' if particle_mode else 'cells'}")

        if keyboard.get_pressed(bsk.key.KeyCode.K_F):
            fire_mode = not fire_mode
            print(f"Fire mode: {'ON' if fire_mode else 'OFF'}")

        if keyboard.get_pressed(bsk.key.KeyCode.K_Z):
            static_mode = not static_mode
            print(f"Static mode: {'ON' if static_mode else 'OFF'}")

        if mouse.get_left_down():
            ok, pixel_x, pixel_y = cell_buffer.world_to_pixel(mouse_pos)
            if ok and 0 <= pixel_x < buffer_width and 0 <= pixel_y < buffer_height:
                brush = rainbow_brush_color(engine, mat_id, fire_mode)
                brush.mat_id = (brush.mat_id & 0x0F) | (STATIC_MAT_BIT if static_mode else 0)
                if particle_mode:
                    cell_buffer.apply_particle_brush(pixel_x, pixel_y, buffer_height // 50, 64, brush)
                else:
                    cell_buffer.apply_brush(pixel_x, pixel_y, buffer_height // 100, brush)

        if keyboard.get_pressed(bsk.key.K_B):
            bsk.Node2D(scene, None, None, mouse_pos, collider=collider)

        camera_pos = scene.get_camera().get_position()

        cell_buffer.simulate(float(engine.get_delta_time()))
        cell_buffer.update_texture()
        scene.render()

        sand_shader.set_uniform("location", camera_pos)
        sand_shader.set_uniform("cameraScale", camera_scale)
        sand_shader.set_uniform("bufferSize", glm.vec2(buffer_width, buffer_height))
        sand_shader.set_uniform("cellScale", cell_buffer.get_cell_scale())
        sand_frame.render(
            cell_buffer.get_render_texture(),
            0,
            0,
            engine.get_window_width(),
            engine.get_window_height(),
        )
        engine.render()


if __name__ == "__main__":
    main()
