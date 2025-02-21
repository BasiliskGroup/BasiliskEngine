import basilisk as bsk

engine = bsk.Engine()
scene = bsk.Scene(engine)

node = bsk.Node()
scene.add(node)

while engine.running:
    scene.update()
    engine.update()