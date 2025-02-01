import basilisk as bsk
import cudart

engine = bsk.Engine()
scene = bsk.Scene()
engine.scene = scene

scene.add(bsk.Node())

while engine.running:
    engine.update()