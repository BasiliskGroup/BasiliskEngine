import basilisk as bsk
from math import cos, sin, pi
import glm
import time
import random

engine = bsk.Engine()
scene = bsk.Scene()
engine.scene = scene

cube_mesh = bsk.Mesh('tests/cube.obj')
sphere_mesh = bsk.Mesh('tests/sphere.obj')

mud = bsk.Image("tests/mud.png")
mud_normal = bsk.Image("tests/mud_normal.png")
mud_mtl = bsk.Material(texture=mud, normal=mud_normal)
materials = [bsk.Material(color=(255 * (i & 4), 255 * (i & 2), 255 * (i & 1))) for i in range(1, 7)]

for i in range(3):
    scene.add_node(
        position=[random.uniform(-10, 10), random.uniform(-3, -10), random.uniform(-10, 10)], 
        # scale=(1, 2, 3),
        mesh=sphere_mesh, 
        material=mud_mtl,
        # velocity=(0, 0.1, 0),
        # rotational_velocity=(0, 1, 0),
        # physics=True,
        # mass=1,
        collisions=True
    )

while engine.running:
    engine.update()