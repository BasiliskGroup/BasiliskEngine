import basilisk as bsk
import pygame as pg

from math import cos, sin

engine = bsk.Engine()
scene = bsk.Scene()
engine.scene = scene


mud = bsk.Image("tests/mud.png")
mud_normal = bsk.Image("tests/mud_normal.png")
mud_mtl = bsk.Material(metallicness=1.0)
cube_mesh = bsk.Mesh('tests/cube.obj')

# engine.shader = bsk.Shader(engine, vert='basilisk/shaders/normal.vert', frag='basilisk/shaders/normal.frag')

n = 10
d = 5

nodes = []

for x in range(-n, n):
    for y in range(-n, n):
        for z in range(-n, n):
            nodes.append(scene.add_node(position=(x * d, y * d, z * d)))


while engine.running:
    nodes[0].x += (engine.keys[pg.K_RIGHT] - engine.keys[pg.K_LEFT]) * engine.delta_time * 5
    engine.update()