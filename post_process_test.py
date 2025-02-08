import basilisk as bsk

engine = bsk.Engine()
scene = bsk.Scene()
engine.scene = scene

# crt = bsk.PostProcess(engine, "basilisk/shaders/crt.frag")
# downscale = bsk.PostProcess(engine, size=(100, 100))
# scene.add(downscale)

scene.add(bsk.Node())

while engine.running:
    engine.update()
