import numpy as np
import pygame as pg
import moderngl as mgl


class StartScreen:
    def __init__(self, scene) -> None:
        self.scene = scene
        self.ctx = scene.ctx
        self.clock = pg.Clock()
        self.win_size = self.scene.engine.win_size

        self.set_renderer()
        self.load_images()

    def draw(self):
        self.ctx.clear()

        # Get the frame from the video player as a pygame surface

        surf = self.bg.copy()
        cursor_pos = pg.mouse.get_pos()
        surf.blit(self.text, (self.win_size[0] / 2 - 500/2, self.win_size[1] / 2 - 150/2))
        surf.blit(self.cursor, (cursor_pos[0] - 60/2, cursor_pos[1] - 100/2))
        surf = pg.transform.flip(surf, False, True)

        self.texture.write(surf.get_view('2'))

        self.program['screenTexture'] = 0
        self.texture.use(location=0)

        self.vao.render()

        pg.display.flip()
        
    def load_images(self):
        # Load the background
        # self.bg = pg.image.load('textures/america.jpg').convert_alpha()
        self.bg = pg.Surface(self.win_size).convert_alpha()
        self.bg.fill((0, 0, 0, 255))

        # Load cursor
        self.cursor = pg.image.load('textures/cursor.png').convert_alpha()
        self.cursor = pg.transform.scale(self.cursor, (75, 100))

        self.text = pg.image.load('textures/clicktostart.png').convert_alpha()
        self.text = pg.transform.scale(self.text, (500, 150))

        # Load the texture
        if self.texture: self.texture.release()
        self.texture = self.ctx.texture(self.bg.get_size(), 4)
        self.texture.filter = (mgl.LINEAR, mgl.LINEAR)
        self.texture.swizzel = 'BGRA'

    def set_renderer(self):
        # MGL render
        self.frame_vertex_data = np.array([-1.0, 1.0, 0.0, 0.0, 1.0, -1.0, -1.0, 0.0, 0.0, 0.0, 1.0, -1.0, 0.0, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0, 1.0, -1.0, 1.0, 0.0, 0.0, 1.0, 1.0, -1.0, 0.0, 1.0, 0.0], dtype='f4')
        self.vbo = self.ctx.buffer(self.frame_vertex_data)

        # Read the shaders
        with open(f'shaders/cutscene.vert') as file:
            vertex_shader   = file.read()
        with open(f'shaders/cutscene.frag') as file:
            fragment_shader = file.read()
        # Load shader program
        self.program = self.ctx.program(vertex_shader=vertex_shader, fragment_shader=fragment_shader)

        # Load vao
        self.vao = self.ctx.vertex_array(self.program, [(self.vbo, '3f 2f', *['in_position', 'in_uv'])], skip_errors=True)

        # Set texture
        self.texture = None

    def start(self):
        # Unlock mouse
        pg.event.set_grab(False)
        pg.mouse.set_visible(False)
        # Start the video player
        self.dt = self.clock.tick() / 1000
        self.run = True
        # Run until the video is done
        while self.run:
            for event in pg.event.get():
                if event.type == pg.QUIT:
                    self.scene.engine.run = False
                    pg.quit()

                if event.type == pg.VIDEORESIZE:
                    self.win_size = (event.w, event.h)
                    self.scene.engine.win_size = (event.w, event.h)
                    self.load_images()

                if pg.mouse.get_pressed()[0]:
                    self.run = False

            self.dt = self.clock.tick() / 1000
            self.draw()