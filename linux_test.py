import basilisk as bsk
import cudart

engine = bsk.Engine()
scene = bsk.Scene(engine)

scene.add(bsk.Node())

while engine.running:
    scene.update()
    engine.update()