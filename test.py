import basilisk as bsk

engine = bsk.Engine()
scene = bsk.Scene()
engine.scene = scene

cube_mesh = bsk.Mesh('cube.obj')

brick = bsk.Image("brick.png")
brick_normal = bsk.Image("brick_normal.png")
brick_mtl = bsk.Material(texture=brick, normal=brick_normal)

scene.add_node(mesh=cube_mesh, material=brick_mtl)

while engine.running:
    engine.update()