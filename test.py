import basilisk as bsk
from math import cos, sin

engine = bsk.Engine()
scene = bsk.Scene()
engine.scene = scene

cube_mesh = bsk.Mesh('tests/cube.obj')
sphere_mesh = bsk.Mesh('tests/sphere.obj')

brick = bsk.Image("tests/brick.png")
brick_normal = bsk.Image("tests/brick_normal.png")
mud = bsk.Image("tests/mud.png")
mud_normal = bsk.Image("tests/mud_normal.png")
brick_mtl = bsk.Material(texture=brick, normal=brick_normal)
mud_mtl = bsk.Material(texture=mud, normal=mud_normal)

test_mtl  = bsk.Material(roughness=0.0 , texture=mud, normal=mud_normal)

scene.add_node(position=(0, 0, 0), mesh=sphere_mesh, material=test_mtl)

while engine.running:
    engine.update()