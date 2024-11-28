import basilisk as bsk
from math import cos, sin

engine = bsk.Engine()
scene = bsk.Scene()
engine.scene = scene

cube_mesh = bsk.Mesh('tests/cube.obj')
sphere_mesh = bsk.Mesh('tests/sphere.obj')

brick = bsk.Image("tests/brick.png")
brick_normal = bsk.Image("tests/brick_normal.png")
brick_mtl = bsk.Material(texture=brick, normal=brick_normal)

scene.add_node(mesh=cube_mesh, material=brick_mtl)
scene.add_node(position = (5, 0, 0), mesh=sphere_mesh, material=brick_mtl)

while engine.running:
    scene.light_handler.directional_light.direction = (cos(engine.time), -1, sin(engine.time))
    engine.update()