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

        self.sphere_mesh = bsk.Mesh('tests/sphere.obj')
        self.monkey_mesh = bsk.Mesh('tests/monkey.obj')

        self.mud = bsk.Image('tests/mud.png')
        self.mud_normal = bsk.Image('tests/mud_normal.png')
        self.cloth_albedo = bsk.Image('tests/cloth_albedo.png')
        self.cloth_normal = bsk.Image('tests/cloth_normal.png')
        self.foil_normal = bsk.Image('tests/foil_normal.png')
        self.floor_albedo = bsk.Image('tests/floor_albedo.png')
        self.floor_normal = bsk.Image('tests/floor_normal.png')

        self.mud_mtl = bsk.Material(texture=self.mud, normal=self.mud_normal)
        self.foil_mtl = bsk.Material(normal=self.foil_normal)
        self.cloth_mtl = bsk.Material(texture=self.cloth_albedo, normal=self.cloth_normal)
        self.floor_mtl = bsk.Material(texture=self.floor_albedo, normal=self.floor_normal)

        self.mtl = bsk.Material()
       
        self.show_menu = True
       
        self.node = self.scene.add_node(mesh=self.sphere_mesh, material=self.mtl)

        self.base_sky = bsk.Sky(self.engine, 'tests/skybox.png')
        self.sunset_sky = bsk.Sky(self.engine, 'tests/SkySkybox.png')

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
        if self.engine.keys[pg.K_3]:
            self.engine.mouse.set_pos(400, 400)
        if self.engine.keys[pg.K_m]:
            self.node.material = self.mud_mtl
            self.mtl = self.mud_mtl
        if self.engine.keys[pg.K_f]:
            self.node.material = self.foil_mtl
            self.mtl = self.foil_mtl
        if self.engine.keys[pg.K_c]:
            self.node.material = self.cloth_mtl
            self.mtl = self.cloth_mtl
        if self.engine.keys[pg.K_g]:
            self.node.material = self.floor_mtl
            self.mtl = self.floor_mtl
        if self.engine.keys[pg.K_5]:
            self.scene.sky = self.base_sky
        if self.engine.keys[pg.K_6]:
            self.scene.sky = self.sunset_sky
        if self.engine.keys[pg.K_p]:
            self.scene.frame.save()

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