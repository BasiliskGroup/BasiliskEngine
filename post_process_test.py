import basilisk as bsk
import moderngl as mgl

engine = bsk.Engine()
scene = bsk.Scene(engine)

crt = bsk.PostProcess(engine, 'basilisk/shaders/crt.frag')
scene.add(crt)
scene.add(bsk.Node())

while engine.running:
    scene.update()
    engine.update()


