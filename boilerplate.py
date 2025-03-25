import basilisk as bsk

engine = bsk.Engine(title=None)
scene = bsk.Scene(engine)

scene.add(bsk.Node())

while engine.running:
    scene.update()
    engine.update(render=False)