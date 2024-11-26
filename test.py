import basiliskengine as bsk

engine = bsk.Engine()
scene = bsk.Scene()
engine.scene = scene

while engine.running:
    engine.update()