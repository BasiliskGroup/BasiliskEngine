import basilisk as bsk

engine = bsk.Engine(title=None)
scene = bsk.Scene(engine)

scene.add(bsk.Node(scale=(10, .5, 10)))

while engine.running:
    scene.update()
    engine.update()