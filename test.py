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

r_0  = bsk.Material(roughness=0.0 , texture=mud, normal=mud_normal)
r_25 = bsk.Material(roughness=0.25, texture=mud, normal=mud_normal)
r_50 = bsk.Material(roughness=0.5 , texture=mud, normal=mud_normal)
r_75 = bsk.Material(roughness=0.75, texture=mud, normal=mud_normal)
r_1  = bsk.Material(roughness=1.0 , texture=mud, normal=mud_normal)

scene.add_node(position=(-10, 0, 0), mesh=sphere_mesh, material=r_0)
scene.add_node(position=( -5, 0, 0), mesh=sphere_mesh, material=r_25)
scene.add_node(position=(  0, 0, 0), mesh=sphere_mesh, material=r_50)
scene.add_node(position=(  5, 0, 0), mesh=sphere_mesh, material=r_75)
scene.add_node(position=( 10, 0, 0), mesh=sphere_mesh, material=r_1)

while engine.running:
    scene.light_handler.directional_light.direction = (cos(engine.time), -.5, sin(engine.time))
    engine.update()