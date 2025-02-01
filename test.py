import basilisk as bsk

engine = bsk.Engine()
scene = bsk.Scene()
engine.scene = scene

image = bsk.Image('tests/floor_albedo.png')
mtl = bsk.Material(texture=image)
mesh = bsk.Mesh('tests/sphere.obj')

node = bsk.Node(mesh=mesh, scale=(5, 5, 5), material=mtl)
scene.add(node)

while engine.running:
    engine.update()