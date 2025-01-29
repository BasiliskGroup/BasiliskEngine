import basilisk as bsk
import glm
from math import cos, sin
import pygame as pg
import cudart

class App():
    def __init__(self):
        self.engine = bsk.Engine(grab_mouse=False)
        self.scene = bsk.Scene()
        self.engine.scene = self.scene

        self.scene.camera = bsk.StaticCamera(position=(0, 0, 4))
        self.show_menu = True

        self.sphere_mesh = bsk.Mesh('tests/sphere.obj')

        texture = bsk.Image('tests/grass.jpg')
        normal = bsk.Image('tests/grass_normal.png')
        roughness = bsk.Image('tests/grass_roughness.jpg')
        ao = bsk.Image('tests/grass_ao.jpg')
        self.mtl = bsk.Material(texture=texture, normal=normal, roughness_map=roughness, ao_map=ao)
       
        self.node = bsk.Node(mesh=self.sphere_mesh, material=self.mtl)
        self.node = self.scene.add(self.node)

        self.cam_rot = 0

    def update(self):
        if self.engine.keys[pg.K_1]:
            self.scene.camera = bsk.StaticCamera(position=(0, 0, 4))
            self.engine.mouse.grab = False
            self.show_menu = True
        if self.engine.keys[pg.K_2]:
            self.scene.camera = bsk.FreeCamera(position=(0, 0, 4))
            self.engine.mouse.grab = True
            self.show_menu = False
        if self.engine.keys[bsk.pg.K_p]:
            m = self.mtl
            print(f'bsk.Material(roughness={m.roughness}, subsurface={m.subsurface}, sheen={m.sheen}, sheen_tint={m.sheen_tint}, anisotropic={m.anisotropic}, specular={m.specular}, metallicness={m.metallicness}, specular_tint={m.specular_tint}, clearcoat={m.clearcoat}, clearcoat_gloss={m.clearcoat_gloss})')
        

        if self.engine.mouse.left_down:
            if 30 < self.engine.mouse.y < 60:
                self.cam_rot = min(max((self.engine.mouse.x - 90) / 200, 0.0), 1.0)
                self.scene.light_handler.directional_lights[0].direction = (cos(self.cam_rot * 6.28), -.5, sin(self.cam_rot * 6.28))
            if 60 < self.engine.mouse.y < 90:
                self.mtl.roughness = min(max((self.engine.mouse.x - 90) / 200, 0.0), 1.0)
            if 90 < self.engine.mouse.y < 120:
                self.mtl.sheen = min(max((self.engine.mouse.x - 90) / 200, 0.0), 1.0)
            if 120 < self.engine.mouse.y < 150:
                self.mtl.subsurface = min(max((self.engine.mouse.x - 90) / 200, 0.0), 1.0)
            if 150 < self.engine.mouse.y < 180:
                self.mtl.metallicness = min(max((self.engine.mouse.x - 90) / 200, 0.0), 1.0)
            if 180 < self.engine.mouse.y < 210:
                self.mtl.anisotropic = min(max((self.engine.mouse.x - 90) / 200, 0.0), 1.0)
            if 210 < self.engine.mouse.y < 240:
                self.mtl.specular = min(max((self.engine.mouse.x - 90) / 200 * 2, 0.0), 2.0)
            if 240 < self.engine.mouse.y < 270:
                self.mtl.specular_tint = min(max((self.engine.mouse.x - 90) / 200, 0.0), 1.0)
            if 270 < self.engine.mouse.y < 300:
                self.mtl.sheen_tint = min(max((self.engine.mouse.x - 90) / 200, 0.0), 1.0)
            if 300 < self.engine.mouse.y < 330:
                self.mtl.clearcoat = min(max((self.engine.mouse.x - 90) / 200, 0.0), 1.0)
            if 330 < self.engine.mouse.y < 360:
                self.mtl.clearcoat_gloss = min(max((self.engine.mouse.x - 90) / 200, 0.0), 1.0)

    def draw(self):
        self.draw_slider(1, self.cam_rot, title="Light")
        self.draw_slider(2, self.mtl.roughness, title="Roughness")
        self.draw_slider(3, self.mtl.sheen, title="Sheen")
        self.draw_slider(4, self.mtl.subsurface, title="Subsurface")
        self.draw_slider(5, self.mtl.metallicness, title="Metallicness")
        self.draw_slider(6, self.mtl.anisotropic, title="Anisotropic")
        self.draw_slider(7, self.mtl.specular / 2, title="Specular")
        self.draw_slider(8, self.mtl.specular_tint, title="Specular Tint")
        self.draw_slider(9, self.mtl.sheen_tint, title="Sheen Tint")
        self.draw_slider(10, self.mtl.clearcoat, title="Clearcoat")
        self.draw_slider(11, self.mtl.clearcoat_gloss, title="Clearcoat Gloss")

    def draw_slider(self, y, value, title=None):
        if title: bsk.draw.text(self.engine, title, (45, y * 30 + 15), scale=0.25)
        bsk.draw.rect(self.engine, (100, 100, 100, 150), (90, y * 30 + 10, 200, 10))
        bsk.draw.rect(self.engine, (200, 200, 200, 150), (90 + value * 200, y * 30 + 5, 10, 20))

    def start(self):
        while self.engine.running:
            self.update()
            if self.show_menu: self.draw()
            self.engine.update()

app = App()
app.start()