import basilisk as bsk

from math import cos, sin

engine = bsk.Engine()
scene = bsk.Scene()
engine.scene = scene

cube_mesh = bsk.Mesh('tests/cube.obj')

mud = bsk.Image("tests/mud.png")
mud_normal = bsk.Image("tests/mud_normal.png")
mud_mtl = bsk.Material(texture=mud, normal=mud_normal)

n = 15
d = 8

for x in range(-n, n):
    for y in range(-n, n):
        for z in range(-n, n):
            scene.add_node(position=(x * d, y * d, z * d), mesh=cube_mesh, material=mud_mtl)

while engine.running:
    engine.update()