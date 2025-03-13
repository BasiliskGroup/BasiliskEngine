import basilisk as bsk

engine = bsk.Engine()
scene = bsk.Scene(engine)

scene.add(bsk.Node())

while engine.running:
    scene.update()
    engine.update()