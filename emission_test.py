import basilisk as bsk

engine = bsk.Engine()
scene = bsk.Scene(engine)
# scene.sky = None

emissive_mtl = bsk.Material(emissive_color=(155, 10, 255))


cube = bsk.Node(material=emissive_mtl)
scene.add(cube)

while engine.running:
    scene.update()
    engine.update()