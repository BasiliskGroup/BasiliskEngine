import basilisk as bsk

engine = bsk.Engine()
scene = bsk.Scene()
engine.scene = scene

node = bsk.Node()
scene.add(node)

while engine.running:
    engine.update()