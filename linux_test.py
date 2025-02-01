import basilisk as bsk
import cudart

engine = bsk.Engine(vsync=True)
scene = bsk.Scene()
engine.scene = scene

scene.add(bsk.Node())

while engine.running:
    engine.update()