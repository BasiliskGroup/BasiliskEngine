import basiliskengine as bsk

engine = bsk.Engine()
scene = bsk.Scene()
engine.scene = scene

node = bsk.Node()

while engine.running:
    engine.update()