import numpy as np
import pygame as pg
import glm
import moderngl as mgl
from scripts.video.video import VideoPlayer


class CutsceneHandler:
    def __init__(self, scene) -> None:
        self.scene = scene
        self.ctx = scene.ctx
        self.clock = pg.Clock()

        self.video_player = VideoPlayer()

        self.set_renderer()

    def draw(self):
        self.ctx.clear()

        # Get the frame from the video player as a pygame surface
        frame = self.video_player.get_frame(self.dt)
        if frame:
            if self.texture: self.texture.release()

            self.texture = self.ctx.texture(frame.get_size(), 4)
            self.texture.filter = (mgl.LINEAR, mgl.LINEAR)
            self.texture.swizzel = 'BGR'
            frame = pg.transform.flip(frame, False, True)
            self.texture.write(frame.get_view('2'))

            self.program['screenTexture'] = 0
            self.texture.use(location=0)

            self.vao.render()

        pg.display.flip()

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

    def play_cutscene(self, cutscene_name):
        # Start the video player
        self.dt = self.clock.tick() / 1000
        self.video_player.play("videos/" + cutscene_name + ".mp4", "videos/" + cutscene_name + ".mp3")        
        # Run until the video is done
        while self.video_player.is_playing:
            for event in pg.event.get():
                if event.type == pg.QUIT:
                    self.video_player.stop()
                    self.scene.engine.run = False
                    pg.quit()

                if event.type == pg.VIDEORESIZE:
                    self.win_size = (event.w, event.h)

            self.dt = self.clock.tick() / 1000
            self.draw()