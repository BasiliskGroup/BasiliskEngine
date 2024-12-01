import basilisk as bsk
from math import cos, sin, pi

engine = bsk.Engine()
scene = bsk.Scene()
engine.scene = scene

cube_mesh = bsk.Mesh('tests/cube.obj')
sphere_mesh = bsk.Mesh('tests/sphere.obj')

mud = bsk.Image("tests/mud.png")
mud_normal = bsk.Image("tests/mud_normal.png")
mud_mtl = bsk.Material(texture=mud, normal=mud_normal)

scene.add_node(
    position=(0, 0, -1), 
    mesh=sphere_mesh, 
    material=mud_mtl,
    velocity=(0, 0.1, 0),
    rotational_velocity=(0, 1, 0),
    physics=True
)

while engine.running:
    engine.update()